#include "character/character.h"

#include "character/dynasty.h"
#include "character/item.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "culture/culture_supergroup.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "game/engine_interface.h"
#include "game/game.h"
#include "history/history.h"
#include "holding/holding.h"
#include "landed_title/landed_title.h"
#include "landed_title/landed_title_tier.h"
#include "politics/government_type.h"
#include "script/condition/condition_check.h"
#include "script/context.h"
#include "script/decision/filter/decision_filter.h"
#include "script/decision/holding_decision.h"
#include "script/event/character_event.h"
#include "script/event/event_trigger.h"
#include "script/modifier.h"
#include "util/container_util.h"
#include "util/random.h"
#include "util/string_util.h"
#include "util/vector_util.h"
#include "util/vector_random_util.h"

#include <QVariant>

namespace metternich {

std::set<std::string> character::get_database_dependencies()
{
	return {
		//so that random holders will have been generated when all characters have their history initialized
		landed_title::class_identifier
	};
}

void character::remove(character *character)
{
	if (character->is_alive()) {
		vector::remove(character::living_characters, character);
	}

	data_type<metternich::character>::remove(character);
}

void character::purge_null_characters()
{
	vector::remove(character::living_characters, nullptr);
}

character *character::generate(metternich::culture *culture, metternich::religion *religion, metternich::phenotype *phenotype)
{
	if (culture == nullptr) {
		throw std::runtime_error("Tried to generate a character with no culture.");
	}

	if (religion == nullptr) {
		throw std::runtime_error("Tried to generate a character with no religion.");
	}

	const std::string identifier = character::generate_identifier();
	character *character = character::add(identifier);
	character->culture = culture;
	character->religion = religion;
	if (phenotype != nullptr) {
		character->phenotype = phenotype;
	}
	character->dynasty = dynasty::generate(culture); //generate a new dynasty for the character

	//generate the character's birth date to be between 60 and 20 years before the current date
	const QDateTime &current_date = game::get()->get_current_date();
	character->birth_date = current_date.addDays(random::generate_in_range(-60 * 365, -20 * 365));
	if (!history::get()->is_loading()) { //if history is loading then this entry's history will already be initialized later on anyway
		character->initialize_history(); //generates a name and sets the phenotype if none was given
	}
	return character;
}

character::character(const std::string &identifier) : data_entry(identifier)
{
	connect(this, &character::name_changed, this, &character::full_name_changed);
	connect(this, &character::name_changed, this, &character::titled_name_changed);
	connect(this, &character::dynasty_changed, this, &character::full_name_changed);
	connect(this, &character::primary_title_changed, this, &character::titled_name_changed);
	connect(this, &character::government_type_changed, this, &character::titled_name_changed);

	character::living_characters.push_back(this);
}

character::~character()
{
	//remove references from other characters to his one; necessary since this character could be purged e.g. if it was born after the start date
	if (this->get_father() != nullptr) {
		vector::remove(this->get_father()->children, this);
	}

	if (this->get_mother() != nullptr) {
		vector::remove(this->get_mother()->children, this);
	}

	if (this->get_spouse() != nullptr) {
		this->get_spouse()->spouse = nullptr;
	}

	for (character *child : this->children) {
		if (this->is_female()) {
			child->mother = nullptr;
		} else {
			child->father = nullptr;
		}
	}

	if (this->get_liege() != nullptr) {
		vector::remove(this->get_liege()->vassals, this);
	}

	for (character *vassal : this->vassals) {
		vassal->liege = nullptr;
	}
}

void character::process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
{
	if (property.get_key() == "birth") {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.get_key() + "\" property.");
		}

		if (string::to_bool(property.get_value())) {
			this->birth_date = date;
			this->alive = true;
		}
	} else if (property.get_key() == "death") {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.get_key() + "\" property.");
		}

		if (string::to_bool(property.get_value())) {
			this->death_date = date;
			this->alive = false;
		}
	} else {
		this->process_gsml_property(property);
	}
}

void character::initialize_history()
{
	if (history::get()->is_loading()) {
		if (this->get_culture() != nullptr && !this->name.empty()) {
			//add the character's name to its culture's name list
			if (this->is_female()) {
				this->get_culture()->add_female_name(this->name);
				this->get_culture()->get_group()->add_female_name(this->name);
				this->get_culture()->get_supergroup()->add_female_name(this->name);
			} else {
				this->get_culture()->add_male_name(this->name);
				this->get_culture()->get_group()->add_male_name(this->name);
				this->get_culture()->get_supergroup()->add_male_name(this->name);
			}
		}
	}

	if (this->get_culture() != nullptr) {
		if (this->get_phenotype() == nullptr) {
			this->phenotype = this->get_culture()->get_default_phenotype();
		}

		if (this->name.empty()) {
			if (this->is_female()) {
				this->name = this->get_culture()->generate_female_name();
			} else {
				this->name = this->get_culture()->generate_male_name();
			}
		}
	}

	if (!this->has_personality_trait()) {
		this->generate_personality_trait();
	}

	this->calculate_government_type();

	data_entry_base::initialize_history();
}

void character::do_month()
{
	//do character events
	character_event_trigger::monthly_pulse->do_events(this);

	if (this->is_landed()) {
		character_event_trigger::landed_monthly_pulse->do_events(this);

		if (this->is_ai()) {
			holding_decision_filter::owned->do_ai_decisions(this->holdings, this);
		}
	}
}

void character::do_year()
{
	character_event_trigger::yearly_pulse->do_events(this);

	if (this->is_landed()) {
		character_event_trigger::landed_yearly_pulse->do_events(this);
	}
}

void character::set_name(const std::string &name)
{
	if (name == this->name) {
		return;
	}

	this->name = name;
	emit name_changed();
}

std::string character::get_full_name() const
{
	std::string full_name = this->name;
	if (this->get_dynasty() != nullptr) {
		full_name += " " + this->get_dynasty()->get_name();
	}
	return full_name;
}

std::string character::get_titled_name() const
{
	std::string titled_name;

	if (this->get_primary_title() != nullptr) {
		titled_name += this->get_primary_title()->get_holder_title_name() + " ";
	}

	titled_name += this->get_name();

	if (this->get_primary_title() != nullptr) {
		titled_name += " of " + this->get_primary_title()->get_name();
	}

	return titled_name;
}

void character::set_alive(const bool alive)
{
	if (alive == this->is_alive()) {
		return;
	}

	this->alive = alive;

	if (this->alive) {
		character::living_characters.push_back(this);
	} else {
		auto find_iterator = std::find(character::living_characters.begin(), character::living_characters.end(), this);
		*find_iterator = nullptr;
	}

	emit alive_changed();

	if (!this->is_ai()) {
		engine_interface::get()->add_notification("You died.");
	}
}

void character::set_culture(metternich::culture *culture)
{
	if (culture == this->get_culture()) {
		return;
	}

	this->culture = culture;
	emit culture_changed();
}

void character::set_primary_title(landed_title *title)
{
	if (title == this->get_primary_title()) {
		return;
	}

	landed_title *old_title = this->get_primary_title();
	const province *old_capital_province = this->get_capital_province();
	const province *old_location = this->get_location();

	if (old_title != nullptr) {
		disconnect(old_title, &landed_title::capital_province_changed, this, &character::capital_province_changed);
	} else {
		if (this->get_liege() != nullptr) {
			disconnect(this->get_liege(), &character::capital_province_changed, this, &character::capital_province_changed);
		}
	}

	this->primary_title = title;

	if (old_title != nullptr) {
		if (title != nullptr) {
			title->copy_title_laws_if_missing(old_title);
		}
		old_title->clear_non_succession_laws();
	}

	if (title != nullptr) {
		title->set_missing_laws_to_default();
	}

	emit primary_title_changed();

	if (!history::get()->is_loading()) {
		if (title == nullptr || old_title == nullptr) {
			this->calculate_government_type();
		}
	}

	if (title != nullptr) {
		connect(title, &landed_title::capital_province_changed, this, &character::capital_province_changed);
	} else {
		if (this->get_liege() != nullptr) {
			connect(this->get_liege(), &character::capital_province_changed, this, &character::capital_province_changed);
		}
	}

	if (old_capital_province != this->get_capital_province()) {
		emit capital_province_changed();
	}

	if (old_location != this->get_location()) {
		emit location_changed();

		for (character *vassal : this->vassals) {
			if (vassal->get_primary_title() == nullptr) {
				emit vassal->location_changed();
			}
		}
	}
}

void character::choose_primary_title()
{
	landed_title *best_title = nullptr;
	landed_title_tier best_title_tier = landed_title_tier::barony;

	for (landed_title *title : this->get_landed_titles()) {
		if (best_title == nullptr || title->get_tier() > best_title_tier) {
			best_title = title;
			best_title_tier = title->get_tier();
		}
	}

	this->set_primary_title(best_title);
}

void character::add_landed_title(landed_title *title)
{
	this->landed_titles.push_back(title);

	//if the new title belongs to a higher tier than the current primary title (or if there is no current primary title), change the character's primary title to the new title
	if (this->get_primary_title() == nullptr || this->get_primary_title()->get_tier() < title->get_tier()) {
		this->set_primary_title(title);
	}
}

void character::remove_landed_title(landed_title *title)
{
	vector::remove(this->landed_titles, title);

	if (title == this->get_primary_title()) {
		this->choose_primary_title(); //needs to choose a new primary title, as the old one has been lost
	}
}

void character::remove_holding(holding *holding)
{
	vector::remove(this->holdings, holding);
}

void character::set_father(character *father)
{
	if (this->get_father() == father) {
		return;
	}

	if (this->get_father() != nullptr) {
		vector::remove(this->get_father()->children, this);
	}

	this->father = father;
	father->children.push_back(this);
}

void character::set_mother(character *mother)
{
	if (this->get_mother() == mother) {
		return;
	}

	if (this->get_mother() != nullptr) {
		vector::remove(this->get_mother()->children, this);
	}

	this->mother = mother;
	mother->children.push_back(this);
}

void character::set_liege(character *liege)
{
	if (this->get_liege() == liege) {
		return;
	}

	const province *old_capital_province = this->get_capital_province();
	const province *old_location = this->get_location();

	if (this->get_liege() != nullptr) {
		vector::remove(this->get_liege()->vassals, this);

		if (!this->is_landed()) {
			disconnect(this->get_liege(), &character::capital_province_changed, this, &character::capital_province_changed);
		}
	}

	this->liege = liege;
	emit liege_changed();

	if (liege != nullptr) {
		liege->vassals.push_back(this);

		if (!this->is_landed()) {
			connect(liege, &character::capital_province_changed, this, &character::capital_province_changed);
		}
	}

	if (old_capital_province != this->get_capital_province()) {
		emit capital_province_changed();
	}
	if (old_location != this->get_location()) {
		emit location_changed();
	}
}

province *character::get_capital_province() const
{
	if (this->get_primary_title() != nullptr) {
		return this->get_primary_title()->get_capital_province();
	} else if (this->get_liege() != nullptr) {
		return this->get_liege()->get_capital_province();
	}

	return nullptr;
}

QVariantList character::get_traits_qvariant_list() const
{
	return container::to_qvariant_list(this->get_traits());
}

void character::add_trait(trait *trait)
{
	if (this->has_trait(trait)) {
		return;
	}

	this->traits.push_back(trait);
	if (trait->get_modifier() != nullptr) {
		trait->get_modifier()->apply(this);
	}
	emit traits_changed();

	if (!this->is_ai()) {
		engine_interface::get()->add_notification("You gained the " + string::highlight(trait->get_name()) + " trait.");
	}
}

void character::remove_trait(trait *trait)
{
	if (!this->has_trait(trait)) {
		return;
	}

	vector::remove(this->traits, trait);
	if (trait->get_modifier() != nullptr) {
		trait->get_modifier()->remove(this);
	}
	emit traits_changed();

	if (!this->is_ai()) {
		engine_interface::get()->add_notification("You lost the " + string::highlight(trait->get_name()) + " trait.");
	}
}

bool character::has_personality_trait() const
{
	for (const trait *trait : this->get_traits()) {
		if (trait->is_personality()) {
			return true;
		}
	}

	return false;
}

void character::generate_personality_trait()
{
	std::vector<trait *> potential_traits;

	for (trait *trait : trait::get_personality_traits()) {
		potential_traits.push_back(trait);
	}

	if (potential_traits.empty()) {
		throw std::runtime_error("Could not generate personality trait for character \"" + this->get_identifier() + "\".");
	}

	trait *chosen_trait = vector::get_random(potential_traits);
	this->add_trait(chosen_trait);
}

QVariantList character::get_items_qvariant_list() const
{
	return container::to_qvariant_list(this->get_items());
}

void character::add_item(item *item)
{
	if (this->has_item(item)) {
		return;
	}

	this->items.push_back(item);
	emit items_changed();

	if (!this->is_ai()) {
		engine_interface::get()->add_notification("You gained one " + string::highlight(item->get_name()) + ".");
	}
}

void character::remove_item(item *item)
{
	if (!this->has_item(item)) {
		return;
	}

	vector::remove_one(this->items, item);
	emit items_changed();

	if (!this->is_ai()) {
		engine_interface::get()->add_notification("You lost one " + string::highlight(item->get_name()) + ".");
	}
}

void character::set_government_type(metternich::government_type *government_type)
{
	if (government_type == this->get_government_type()) {
		return;
	}

	this->government_type = government_type;

	if (this->is_landed() && government_type != nullptr) {
		//make the government type be recalculated if a relevant variable changes; only do this for landed characters, as unlanded ones just have the same government as their liege
		this->government_condition_check = std::make_unique<condition_check<character>>(government_type->get_conditions(), this, [this](bool result){
			if (!result) {
				this->calculate_government_type();
			}
		});
	} else {
		this->government_condition_check.reset();
	}

	emit government_type_changed();

	if (!this->get_landed_titles().empty()) {
		for (landed_title *title : this->get_landed_titles()) {
			emit title->government_type_changed();
		}
	}
}

void character::calculate_government_type()
{
	if (!this->is_alive()) {
		this->set_government_type(nullptr);
		return;
	}

	if (!this->is_landed()) {
		if (this->get_liege() != nullptr) {
			this->set_government_type(this->get_liege()->get_government_type());
		} else {
			this->set_government_type(nullptr);
		}
		return;
	}

	read_only_context ctx;
	ctx.current_character = this;

	for (metternich::government_type *government_type : government_type::get_all()) {
		if (government_type->get_conditions() == nullptr || government_type->get_conditions()->check(this, ctx)) {
			this->set_government_type(government_type);
			return;
		}
	}

	throw std::runtime_error("No valid government type for character \"" + this->get_full_name() + "\"" + " (\"" + this->get_identifier() + "\").");
}

bool character::has_law(law *law) const
{
	if (this->get_primary_title() == nullptr) {
		return false;
	}

	//check whether the character's primary title has the law
	return this->get_primary_title()->has_law(law);
}

bool character::is_ai() const
{
	return game::get()->get_player_character() != this;
}

bool character::can_build_in_holding(const holding *holding)
{
	return holding->get_owner() == this || this->is_any_liege_of(holding->get_owner());
}

bool character::can_build_in_holding(const QVariant &holding_variant)
{
	QObject *holding_object = qvariant_cast<QObject *>(holding_variant);
	const holding *holding = static_cast<metternich::holding *>(holding_object);

	if (holding->get_owner() == nullptr) {
		qCritical() << QString::fromStdString("Holding \"" + holding->get_identifier() + "\" has no owner.");
		return false;
	}

	return this->can_build_in_holding(holding);
}

QVariantList character::get_targeted_decisions(const QVariant &target_variant)
{
	QObject *target_object = qvariant_cast<QObject *>(target_variant);
	const std::string class_name = target_object->metaObject()->className();

	if (class_name == "metternich::holding") {
		const holding *holding = static_cast<metternich::holding *>(target_object);
		return this->get_targeted_decisions<holding_decision>(holding);
	} else {
		qCritical() << QString::fromStdString("Invalid targeted decision scope class: \"" + class_name + "\".");
	}

	return QVariantList();
}

}
