#include "landed_title/landed_title.h"

#include "character/character.h"
#include "character/dynasty.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "defines.h"
#include "economy/trade_node.h"
#include "economy/trade_route.h"
#include "game/game.h"
#include "history/history.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title_tier.h"
#include "map/map.h"
#include "map/map_mode.h"
#include "map/province.h"
#include "map/star_system.h"
#include "map/world.h"
#include "politics/government_type.h"
#include "politics/government_type_group.h"
#include "politics/law.h"
#include "politics/law_group.h"
#include "religion/religion.h"
#include "religion/religion_group.h"
#include "translator.h"
#include "util/container_util.h"
#include "util/map_util.h"

#include <stdexcept>

namespace metternich {

const std::vector<landed_title *> &landed_title::get_tier_titles(const landed_title_tier tier)
{
	static std::vector<landed_title *> empty_vector;

	auto find_iterator = landed_title::titles_by_tier.find(tier);
	if (find_iterator != landed_title::titles_by_tier.end()) {
		return find_iterator->second;
	}

	return empty_vector;
}

landed_title *landed_title::add(const std::string &identifier)
{
	landed_title *title = data_type<landed_title>::add(identifier);

	std::string identifier_prefix;
	const size_t find_pos = identifier.find("_");
	if (find_pos != std::string::npos) {
		identifier_prefix = identifier.substr(0, find_pos + 1);
	}

	//set the title's tier depending on the prefix of its identifier
	if (identifier_prefix == landed_title::barony_prefix) {
		title->tier = landed_title_tier::barony;
	} else if (identifier_prefix == landed_title::county_prefix) {
		title->tier = landed_title_tier::county;
	} else if (identifier_prefix == landed_title::duchy_prefix) {
		title->tier = landed_title_tier::duchy;
	} else if (identifier_prefix == landed_title::kingdom_prefix) {
		title->tier = landed_title_tier::kingdom;
	} else if (identifier_prefix == landed_title::empire_prefix) {
		title->tier = landed_title_tier::empire;
	} else {
		throw std::runtime_error("Invalid identifier for new landed title: \"" + identifier + "\". Landed title identifiers must begin with a valid prefix, which depends on the title's tier.");
	}

	landed_title::titles_by_tier[title->get_tier()].push_back(title);

	return title;
}

const char *landed_title::get_tier_identifier(const landed_title_tier tier)
{
	switch (tier) {
		case landed_title_tier::barony: return landed_title::barony_identifier;
		case landed_title_tier::county: return landed_title::county_identifier;
		case landed_title_tier::duchy: return landed_title::duchy_identifier;
		case landed_title_tier::kingdom: return landed_title::kingdom_identifier;
		case landed_title_tier::empire: return landed_title::empire_identifier;
	}

	throw std::runtime_error("Invalid landed title tier enumeration value: " + std::to_string(static_cast<int>(tier)) + ".");
}

const char *landed_title::get_tier_holder_identifier(const landed_title_tier tier)
{
	switch (tier) {
		case landed_title_tier::barony: return landed_title::baron_identifier;
		case landed_title_tier::county: return landed_title::count_identifier;
		case landed_title_tier::duchy: return landed_title::duke_identifier;
		case landed_title_tier::kingdom: return landed_title::king_identifier;
		case landed_title_tier::empire: return landed_title::emperor_identifier;
	}

	throw std::runtime_error("Invalid landed title tier enumeration value: " + std::to_string(static_cast<int>(tier)) + ".");
}

std::string landed_title::get_tier_name(const landed_title_tier tier)
{
	return translator::get()->translate(landed_title::get_tier_identifier(tier));
}

void landed_title::process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
{
	Q_UNUSED(date)

	if (property.get_key() == "holder") {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.get_key() + "\" property.");
		}

		if (property.get_value() == "random") {
			this->set_holder(nullptr);
			this->random_holder = true;
			return;
		} else if (property.get_value() == "none") {
			this->set_holder(nullptr);
			return;
		} else {
			const character *holder = character::get(property.get_value());
			if (holder != nullptr && !holder->is_alive()) {
				return;
			}
		}
	}

	this->process_gsml_property(property);
}

void landed_title::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();

	if (tag.substr(0, 2) == landed_title::barony_prefix) {
		landed_title *barony = landed_title::add(tag);
		barony->set_de_jure_liege_title(this);
		database::process_gsml_data(barony, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

void landed_title::initialize()
{
	if (this->get_tier() == landed_title_tier::barony) {
		if (this->get_holding_slot() != nullptr) {
			if (!this->get_holding_slot()->is_initialized()) {
				this->get_holding_slot()->initialize();
			}

			if (this->get_holding_slot()->get_province() != nullptr) {
				this->capital_province = this->get_holding_slot()->get_province();
			} else if (this->get_holding_slot()->get_world() != nullptr) {
				this->capital_world = this->get_holding_slot()->get_world();
			} else {
				throw std::runtime_error("Holding slot \"" + this->get_holding_slot()->get_identifier() + "\" is not located in either a province or a world.");
			}

			//if a non-titular barony has no de jure liege, set it to be the county of its holding slot's province
			if (this->get_de_jure_liege_title() == nullptr) {
				this->set_de_jure_liege_title(this->get_holding_slot()->get_province()->get_county());
			}
		} else if (this->get_de_jure_liege_title() != nullptr && !this->get_de_jure_liege_title()->is_titular()) {
			//set the barony's capital province to its county's province or world
			if (this->get_de_jure_liege_title()->get_province() != nullptr) {
				this->capital_province = this->get_de_jure_liege_title()->get_province();
			} else if (this->get_de_jure_liege_title()->get_world() != nullptr) {
				this->capital_world = this->get_de_jure_liege_title()->get_world();
			}
		}
	}

	if (this->get_capital_territory() == nullptr) {
		throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has no capital territory.");
	}

	data_entry_base::initialize();
}

void landed_title::initialize_history()
{
	if (this->random_holder) {
		culture *culture = nullptr;
		religion *religion = nullptr;

		if (this->get_holding() != nullptr) {
			if (!this->get_holding()->is_history_initialized()) {
				this->get_holding()->initialize_history();
			}

			culture = this->get_holding()->get_culture();
			religion = this->get_holding()->get_religion();
		} else {
			culture = this->get_capital_territory()->get_culture();
			religion = this->get_capital_territory()->get_religion();
		}

		this->set_holder(character::generate(culture, religion));
	}

	if (this->holder_title != nullptr) {
		if (!this->holder_title->is_history_initialized()) {
			this->holder_title->initialize_history();
		}

		if (this->holder_title->get_holder() == nullptr) {
			throw std::runtime_error("Tried to set the \"" + this->holder_title->get_identifier() + "\" holder title for \"" + this->get_identifier() + "\", but the former has no holder.");
		}

		this->set_holder(this->holder_title->get_holder());
		this->holder_title = nullptr;
	}

	if (this->liege_title != nullptr) {
		if (!this->liege_title->is_history_initialized()) {
			this->liege_title->initialize_history();
		}

		if (this->liege_title->get_holder() == nullptr) {
			throw std::runtime_error("Tried to set the \"" + this->liege_title->get_identifier() + "\" liege title for \"" + this->get_identifier() + "\", but the former has no holder.");
		}

		if (this->get_holder() == nullptr) {
			throw std::runtime_error("Tried to set the \"" + this->liege_title->get_identifier() + "\" liege title for \"" + this->get_identifier() + "\", but the latter has no holder.");
		}

		this->get_holder()->set_liege(this->liege_title->get_holder());
		this->liege_title = nullptr;
	}

	data_entry_base::initialize_history();
}

void landed_title::check() const
{
	if (this->get_tier() != landed_title_tier::barony && this->get_world() == nullptr && !this->get_color().isValid()) {
		throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has no valid color.");
	}

	if (this->get_province() != nullptr && this->get_province() != this->get_capital_province()) {
		throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has a different province and capital province.");
	}

	if (this->get_world() != nullptr && this->get_world() != this->get_capital_world()) {
		throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has a different world and capital world.");
	}

	if (this->get_tier() >= landed_title_tier::duchy) {
		this->get_flag_path(); //throws an exception if the flag isn't found
	}
}

void landed_title::check_history() const
{
	if (this->get_capital_territory() == nullptr) {
		throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has no capital territory.");
	}

	if (this->get_holding_slot() != nullptr && this->get_holding_slot()->get_province() != this->get_capital_province()) {
		throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has its holding slot in a different province than its capital province.");
	}

	if (this->get_holding_slot() != nullptr && this->get_holding_slot()->get_world() != this->get_capital_world()) {
		throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has its holding slot in a different province than its capital province.");
	}

	if (this->get_province() != nullptr) {
		if (this->get_tier() != landed_title_tier::county) {
			throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has been assigned to a province, but is not a county.");
		}
	}

	if (this->get_holding_slot() != nullptr) {
		if (this->get_tier() != landed_title_tier::barony) {
			throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has been assigned to a holding slot, but is not a barony.");
		}
	}

	if (this->get_star_system() != nullptr) {
		if (this->get_tier() != landed_title_tier::duchy) {
			throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has been assigned to a star system, but is not a duchy.");
		}
	}

	this->check();
}

std::string landed_title::get_name() const
{
	return translator::get()->translate(this->get_identifier_with_aliases(), this->get_tag_suffix_list_with_fallbacks());
}

std::string landed_title::get_tier_title_name() const
{
	std::vector<std::vector<std::string>> tag_suffix_list_with_fallbacks = this->get_tag_suffix_list_with_fallbacks();

	tag_suffix_list_with_fallbacks.push_back({this->get_identifier()});

	return translator::get()->translate(landed_title::get_tier_identifier(this->get_tier()), tag_suffix_list_with_fallbacks);
}

std::string landed_title::get_titled_name() const
{
	std::string titled_name = this->get_tier_title_name() + " of ";
	titled_name += this->get_name();
	return titled_name;
}

std::string landed_title::get_holder_title_name() const
{
	std::vector<std::vector<std::string>> tag_suffix_list_with_fallbacks = this->get_tag_suffix_list_with_fallbacks();

	tag_suffix_list_with_fallbacks.push_back(this->get_identifier_with_aliases());

	if (this->get_holder() != nullptr && this->get_holder()->is_female()) {
		tag_suffix_list_with_fallbacks.push_back({"female"});
	}

	return translator::get()->translate(landed_title::get_tier_holder_identifier(this->get_tier()), tag_suffix_list_with_fallbacks);
}

std::vector<std::vector<std::string>> landed_title::get_tag_suffix_list_with_fallbacks() const
{
	std::vector<std::vector<std::string>> tag_list_with_fallbacks;

	if (this->get_holding() != nullptr) {
		//for non-titular baronies, use the holding's type for the localization, so that e.g. a city's holder title name will be "mayor", regardless of the character's actual government
		tag_list_with_fallbacks.push_back({this->get_holding()->get_type()->get_identifier()});
	}

	if (this->get_government_type() != nullptr) {
		tag_list_with_fallbacks.push_back({this->get_government_type()->get_identifier(), government_type_group_to_string(this->get_government_type()->get_group())});
	}

	const culture *culture = this->get_culture();
	if (culture != nullptr) {
		tag_list_with_fallbacks.push_back({culture->get_identifier(), culture->get_culture_group()->get_identifier()});
	}

	const religion *religion = this->get_religion();
	if (religion != nullptr) {
		tag_list_with_fallbacks.push_back({religion->get_identifier(), religion->get_religion_group()->get_identifier()});
	}

	if (this->get_holder() != nullptr && this->get_holder()->get_dynasty() != nullptr) {
		//allow for different localizations/flags depending on the title holder's dynasty
		tag_list_with_fallbacks.push_back({this->get_holder()->get_dynasty()->get_identifier()});
	}

	return tag_list_with_fallbacks;
}

void landed_title::set_holder(character *character)
{
	if (character == this->get_holder()) {
		return;
	}

	metternich::character *old_holder = this->get_holder();
	const landed_title *old_realm = this->get_realm();
	if (old_holder != nullptr) {
		old_holder->remove_landed_title(this);
	}

	this->holder = character;

	if (character != nullptr) {
		character->add_landed_title(this);

		if (!this->is_primary()) {
			this->clear_non_succession_laws();
			this->set_missing_law_to_default_for_law_group(defines::get()->get_succession_law_group());
		}
	} else {
		this->laws.clear();
		emit laws_changed();
	}

	this->holder_title = nullptr; //set the holder title to null, so that the new holder (null or otherwise) isn't overwritten by a previous holder title
	this->random_holder = false;

	const landed_title *realm = this->get_realm();

	if (this->get_province() != nullptr) {
		//if this is a non-titular county, then the character holding it must also possess the county's capital holding
		if (this->get_province()->get_capital_holding() != nullptr) {
			this->get_province()->get_capital_holding()->get_barony()->set_holder(character);
		}

		//if this is a non-titular county, the fort, university and hospital of its province must belong to the county holder
		holding *fort_holding = this->get_province()->get_fort_holding_slot()->get_holding();
		if (fort_holding != nullptr) {
			fort_holding->set_owner(character);
		}

		holding *university_holding = this->get_province()->get_university_holding_slot()->get_holding();
		if (university_holding != nullptr) {
			university_holding->set_owner(character);
		}

		holding *hospital_holding = this->get_province()->get_hospital_holding_slot()->get_holding();
		if (hospital_holding != nullptr) {
			hospital_holding->set_owner(character);
		}

		if (this->get_province()->get_trading_post_holding_slot() != nullptr) {
			holding *trading_post_holding = this->get_province()->get_trading_post_holding_slot()->get_holding();
			if (trading_post_holding != nullptr && (trading_post_holding->get_owner() == nullptr || trading_post_holding->get_owner() == old_holder)) {
				trading_post_holding->set_owner(character);
			}
		}

		if (old_holder == nullptr || character == nullptr) {
			if (!history::get()->is_loading()) {
				this->get_province()->calculate_trade_node();
			}

			if (this->get_province()->is_center_of_trade()) {
				this->get_province()->get_trade_node()->set_active(character != nullptr);
			}

			//update the activity of trade routes which pass through this province
			for (trade_route *route : this->get_province()->get_trade_routes()) {
				if (route->is_endpoint(this->get_province())) {
					if (character != nullptr) {
						route->calculate_active();
					} else {
						route->set_active(false);
					}
				}
			}
		} else if (old_realm != realm) {
			this->get_province()->set_trade_node_recalculation_needed(true);
		}
	}

	if (map::get()->get_mode() == map_mode::country && old_realm != realm) {
		if (this->get_province() != nullptr) {
			this->get_province()->update_color_for_map_mode(map::get()->get_mode());
		} else if (this->get_star_system() != nullptr) {
			this->get_star_system()->update_color_for_map_mode(map::get()->get_mode());
		}
	}

	//if this title is associated with a holding (i.e. it is a non-titular barony), then its holder must also be the owner of the holding
	if (this->get_holding() != nullptr) {
		this->get_holding()->set_owner(character);
	}

	emit holder_changed();
}

void landed_title::set_holding_slot(metternich::holding_slot *holding_slot)
{
	if (holding_slot == this->get_holding_slot()) {
		return;
	}

	this->holding_slot = holding_slot;
}

holding *landed_title::get_holding() const
{
	if (this->get_holding_slot() != nullptr) {
		return this->get_holding_slot()->get_holding();
	}

	return nullptr;
}

landed_title *landed_title::get_realm() const
{
	character *holder = this->get_holder();

	if (holder != nullptr) {
		character *top_liege = holder->get_top_liege();
		return top_liege->get_primary_title();
	}

	return nullptr;
}

/**
**	@brief	Get the landed title's (de facto) liege title
**
**	@return	The (de facto) liege title
*/
landed_title *landed_title::get_liege_title() const
{
	character *holder = this->get_holder();

	if (holder != nullptr) {
		landed_title *holder_primary_title = holder->get_primary_title();
		if (holder_primary_title->get_tier() == this->get_tier()) {
			character *liege = holder->get_liege();
			if (liege != nullptr) {
				return liege->get_primary_title();
			}
		} else {
			if (this->get_tier() < landed_title_tier::county && holder_primary_title->get_tier() >= landed_title_tier::county && holder->has_landed_title(this->get_de_jure_county())) {
				return this->get_de_jure_county();
			}

			if (this->get_tier() < landed_title_tier::duchy && holder_primary_title->get_tier() >= landed_title_tier::duchy && holder->has_landed_title(this->get_de_jure_duchy())) {
				return this->get_de_jure_duchy();
			}

			if (this->get_tier() < landed_title_tier::kingdom && holder_primary_title->get_tier() >= landed_title_tier::kingdom && holder->has_landed_title(this->get_de_jure_kingdom())) {
				return this->get_de_jure_kingdom();
			}

			if (this->get_tier() < landed_title_tier::empire && holder_primary_title->get_tier() >= landed_title_tier::empire && holder->has_landed_title(this->get_de_jure_empire())) {
				return this->get_de_jure_empire();
			}

			return holder_primary_title;
		}
	}

	return nullptr;
}

void landed_title::set_holder_title(landed_title *title)
{
	this->set_holder(nullptr); //set the holder title to null, so that the new holder title isn't overwritten by a previous holder
	this->random_holder = false;
	this->holder_title = title;
}

void landed_title::set_de_jure_liege_title(landed_title *title)
{
	if (title == this->get_de_jure_liege_title()) {
		return;
	}

	if (this->get_de_jure_liege_title() != nullptr) {
		this->get_de_jure_liege_title()->remove_de_jure_vassal_title(this);
	}

	if (title != nullptr && static_cast<int>(title->get_tier()) - static_cast<int>(this->get_tier()) != 1) {
		throw std::runtime_error("Tried to set title \"" + title->get_identifier() + "\" as the de jure liege of \"" + this->get_identifier() + "\", but the former is not one title tier above the latter.");
	}

	this->de_jure_liege_title = title;

	if (title != nullptr) {
		title->add_de_jure_vassal_title(this);
	}

	emit de_jure_liege_title_changed();
}

/**
**	@brief	Get the title's (de facto) title for a given tier
**
**	@return	The title's (de facto) title for the given tier
*/
landed_title *landed_title::get_tier_title(const landed_title_tier tier) const
{
	if (this->get_tier() > tier) {
		return nullptr;
	} else if (this->get_tier() == tier) {
		return const_cast<landed_title *>(this);
	}

	for (int i = static_cast<int>(tier) - 1; i >= static_cast<int>(landed_title_tier::barony); --i) {
		landed_title *tier_title = this->get_tier_title(static_cast<landed_title_tier>(i));
		if (tier_title != nullptr) {
			landed_title *liege_title = tier_title->get_liege_title();
			if (liege_title != nullptr && liege_title->get_tier() == tier) {
				return liege_title;
			}

			return nullptr;
		}
	}

	landed_title *liege_title = this->get_liege_title();
	if (liege_title != nullptr && liege_title->get_tier() == tier) {
		return liege_title;
	}

	return nullptr;
}

/**
**	@brief	Get the title's de jure title for a given tier
**
**	@return	The title's de jure title for the given tier
*/
landed_title *landed_title::get_tier_de_jure_title(const landed_title_tier tier) const
{
	if (this->get_tier() > tier) {
		return nullptr;
	} else if (this->get_tier() == tier) {
		return const_cast<landed_title *>(this);
	}

	if (tier > landed_title_tier::barony) {
		landed_title *lower_tier_title = this->get_tier_de_jure_title(static_cast<landed_title_tier>(static_cast<int>(tier) - 1));
		if (lower_tier_title != nullptr) {
			return lower_tier_title->get_de_jure_liege_title();
		}
	}

	return nullptr;
}

/**
**	@brief	Get the title's (de facto) county
**
**	@return	The title's (de facto) county
*/
landed_title *landed_title::get_county() const
{
	return this->get_tier_title(landed_title_tier::county);
}

landed_title *landed_title::get_de_jure_county() const
{
	return this->get_tier_de_jure_title(landed_title_tier::county);
}

/**
**	@brief	Get the title's (de facto) duchy
**
**	@return	The title's (de facto) duchy
*/
landed_title *landed_title::get_duchy() const
{
	return this->get_tier_title(landed_title_tier::duchy);
}

landed_title *landed_title::get_de_jure_duchy() const
{
	return this->get_tier_de_jure_title(landed_title_tier::duchy);
}

/**
**	@brief	Get the title's (de facto) kingdom
**
**	@return	The title's (de facto) kingdom
*/
landed_title *landed_title::get_kingdom() const
{
	return this->get_tier_title(landed_title_tier::kingdom);
}

landed_title *landed_title::get_de_jure_kingdom() const
{
	return this->get_tier_de_jure_title(landed_title_tier::kingdom);
}

/**
**	@brief	Get the title's (de facto) empire
**
**	@return	The title's (de facto) empire
*/
landed_title *landed_title::get_empire() const
{
	return this->get_tier_title(landed_title_tier::empire);
}

landed_title *landed_title::get_de_jure_empire() const
{
	return this->get_tier_de_jure_title(landed_title_tier::empire);
}

bool landed_title::is_primary() const
{
	if (this->get_holder() == nullptr) {
		return false;
	}

	return this->get_holder()->get_primary_title() == this;
}

territory *landed_title::get_capital_territory() const
{
	if (this->get_capital_province() != nullptr) {
		return this->get_capital_province();
	} else if (this->get_capital_world() != nullptr) {
		return this->get_capital_world();
	}

	return nullptr;
}

holding *landed_title::get_capital_holding() const
{
	if (this->get_holding() != nullptr) {
		return this->get_holding();
	}

	if (this->get_capital_territory() != nullptr) {
		return this->get_capital_territory()->get_capital_holding();
	}

	return nullptr;
}

culture *landed_title::get_culture() const
{
	if (this->get_holder() != nullptr) {
		return this->get_holder()->get_culture();
	} else if (this->get_capital_territory() != nullptr) {
		return this->get_capital_territory()->get_culture();
	}

	return nullptr;
}

religion *landed_title::get_religion() const
{
	if (this->get_holder() != nullptr) {
		return this->get_holder()->get_religion();
	} else if (this->get_capital_territory() != nullptr) {
		return this->get_capital_territory()->get_religion();
	}

	return nullptr;
}

const std::filesystem::path &landed_title::get_flag_path() const
{
	std::string base_tag = this->get_flag_tag();

	const std::filesystem::path &flag_path = database::get()->get_tagged_flag_path(base_tag, this->get_tag_suffix_list_with_fallbacks());
	return flag_path;
}

QVariantList landed_title::get_laws_qvariant_list() const
{
	return container::to_qvariant_list(map_container::get_values(this->laws));
}

bool landed_title::has_law(const law *law) const
{
	auto find_iterator = this->laws.find(law->get_group());
	if (find_iterator != this->laws.end() && find_iterator->second == law) {
		return true;
	}

	return false;
}

Q_INVOKABLE void landed_title::add_law(law *law)
{
	if (!this->has_law(law)) {
		this->laws[law->get_group()] = law;
		emit laws_changed();

		if (this->is_primary()) {
			emit this->get_holder()->laws_changed();
		}
	}
}

Q_INVOKABLE void landed_title::remove_law(law *law)
{
	if (this->has_law(law)) {
		this->laws.erase(law->get_group());
		emit laws_changed();

		if (this->is_primary()) {
			emit this->get_holder()->laws_changed();
		}
	}
}

void landed_title::clear_non_succession_laws()
{
	if (this->laws.empty()) {
		return;
	}

	if (this->laws.size() == 1 && this->laws.contains(defines::get()->get_succession_law_group())) {
		//the title already has only a succession law
		return;
	}

	//save the old succession law
	law *succession_law = this->get_law(defines::get()->get_succession_law_group());

	this->laws.clear();

	if (succession_law != nullptr) {
		//restore the succession law
		this->laws[defines::get()->get_succession_law_group()] = succession_law;
	}

	emit laws_changed();

	if (this->is_primary()) {
		emit this->get_holder()->laws_changed();
	}
}

void landed_title::set_missing_laws_to_default()
{
	const holding *capital_holding = this->get_capital_holding();
	if (capital_holding != nullptr) {
		for (const auto &kv_pair : capital_holding->get_type()->get_default_laws()) {
			if (this->laws.contains(kv_pair.first)) {
				continue;
			}

			this->add_law(kv_pair.second);
		}
	}
}

void landed_title::set_missing_law_to_default_for_law_group(law_group *law_group)
{
	if (this->laws.contains(law_group)) {
		return;
	}

	const holding *capital_holding = this->get_capital_holding();
	if (capital_holding != nullptr) {
		const auto &default_laws = capital_holding->get_type()->get_default_laws();
		auto find_iterator = default_laws.find(law_group);

		if (find_iterator != default_laws.end()) {
			this->add_law(find_iterator->second);
			return;
		}
	}
}

government_type *landed_title::get_government_type() const
{
	if (this->get_holder() != nullptr) {
		return this->get_holder()->get_government_type();
	}

	return nullptr;
}

}
