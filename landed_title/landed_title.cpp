#include "landed_title/landed_title.h"

#include "character/character.h"
#include "character/dynasty.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "game/game.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title_tier.h"
#include "map/province.h"
#include "politics/law.h"
#include "religion.h"
#include "translator.h"
#include "util/container_util.h"

#include <stdexcept>

namespace metternich {

/**
**	@brief	Add a new instance of the class
**
**	@param	identifier	The instance's identifier
**
**	@return	The new instance
*/
landed_title *landed_title::add(const std::string &identifier)
{
	landed_title *title = data_type<landed_title>::add(identifier);

	std::string identifier_prefix = identifier.substr(0, 2);

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
		throw std::runtime_error("Invalid identifier for new landed title: \"" + identifier + "\". Landed title identifiers must begin with one of the \"" + landed_title::barony_prefix + "\", \"" + landed_title::county_prefix + "\", \"" + landed_title::duchy_prefix + "\", \"" + landed_title::kingdom_prefix + "\" or \"" + landed_title::empire_prefix + "\" prefixes, depending on the title's tier.");
	}

	return title;
}

/**
**	@brief	Get the string identifier for a landed title tier
**
**	@param	tier	The tier enumeration value
**
**	@return	The string identifier for the title tier
*/
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

/**
**	@brief	Get the string identifier for a landed title tier's holder
**
**	@param	tier	The tier enumeration value
**
**	@return	The string identifier for the title tier's holder
*/
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

/**
**	@brief	Process a GSML history property
**
**	@param	property	The property
**	@param	date		The date of the property change
*/
void landed_title::process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
{
	Q_UNUSED(date)

	if (property.get_key() == "holder") {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for the \"" + property.get_key() + "\" property.");
		}

		if (property.get_value() == "random") {
			//generate random holder
			character *holder = character::generate(this->get_capital_province()->get_culture(), this->get_capital_province()->get_religion());
			this->set_holder(holder);
			return;
		} else if (property.get_value() == "none") {
			this->set_holder(nullptr);
			return;
		} else {
			const character *holder = character::get(std::stoi(property.get_value()));
			if (holder != nullptr && !holder->is_alive()) {
				return;
			}
		}
	}

	this->process_gsml_property(property);
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void landed_title::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for landed titles needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->color.setRgb(red, green, blue);
	} else if (tag.substr(0, 2) == landed_title::barony_prefix) {
		landed_title *barony = landed_title::add(tag);
		barony->set_de_jure_liege_title(this);
		database::process_gsml_data(barony, scope);
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

/**
**	@brief	Initialize the landed title
*/
void landed_title::initialize()
{
	if (this->get_tier() == landed_title_tier::barony) {
		if (this->get_holding_slot() != nullptr) {
			this->capital_province = this->get_holding_slot()->get_province();
		} else if (this->get_de_jure_liege_title() != nullptr && !this->get_de_jure_liege_title()->is_titular()) {
			//set the barony's capital province to its county's province
			this->capital_province = this->get_de_jure_liege_title()->get_province();
		}
	}

	if (this->get_capital_province() == nullptr) {
		throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has no capital province.");
	}
}

/**
**	@brief	Initialize the landed title's history
*/
void landed_title::initialize_history()
{
	if (this->holder_title != nullptr) {
		if (this->holder_title->get_holder() == nullptr) {
			throw std::runtime_error("Tried to set the \"" + this->holder_title->get_identifier() + "\" holder title for \"" + this->get_identifier() + "\", but the former has no holder.");
		}

		this->set_holder(this->holder_title->get_holder());
		this->holder_title = nullptr;
	}

	if (this->liege_title != nullptr) {
		if (this->liege_title->get_holder() == nullptr) {
			throw std::runtime_error("Tried to set the \"" + this->liege_title->get_identifier() + "\" liege title for \"" + this->get_identifier() + "\", but the former has no holder.");
		}

		if (this->get_holder() == nullptr) {
			throw std::runtime_error("Tried to set the \"" + this->liege_title->get_identifier() + "\" liege title for \"" + this->get_identifier() + "\", but the latter has no holder.");
		}

		this->get_holder()->set_liege(this->liege_title->get_holder());
		this->liege_title = nullptr;
	}
}

/**
**	@brief	Check whether the landed title is in a valid state
*/
void landed_title::check() const
{
	if (this->get_tier() != landed_title_tier::barony && !this->get_color().isValid()) {
		throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has no valid color.");
	}

	if (this->get_province() != nullptr && this->get_province() != this->get_capital_province()) {
		throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has a different province and capital province.");
	}

	if (this->get_tier() >= landed_title_tier::duchy) {
		this->get_flag_path(); //throws an exception if the flag isn't found
	}

	if (game::get()->is_starting()) {
		if (this->get_capital_province() == nullptr) {
			throw std::runtime_error("Landed title \"" + this->get_identifier() + "\" has no capital province.");
		}

		if (this->get_holding_slot() != nullptr && this->get_holding_slot()->get_province() != this->get_capital_province()) {
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
	}
}

/**
**	@brief	Get the landed title's name
**
**	@return	The landed title's name
*/
std::string landed_title::get_name() const
{
	const culture *culture = this->get_culture();

	std::vector<std::string> suffixes;

	if (culture != nullptr) {
		suffixes.push_back(culture->get_identifier());
		suffixes.push_back(culture->get_culture_group()->get_identifier());
	}

	if (this->get_holder() != nullptr && this->get_holder()->get_dynasty() != nullptr) {
		//allow for different localizations depending on the title holder's dynasty
		suffixes.push_back(this->get_holder()->get_dynasty()->get_identifier());
	}

	return translator::get()->translate(this->get_identifier(), suffixes);
}

/**
**	@brief	Get the landed title's tier title name
**
**	@return	The landed title's tier title name
*/
std::string landed_title::get_tier_title_name() const
{
	const culture *culture = this->get_culture();

	std::vector<std::string> suffixes;

	if (this->get_holding() != nullptr) {
		//for non-titular baronies, use the holding's type for the localization
		suffixes.push_back(this->get_holding()->get_type()->get_identifier());
	}

	if (culture != nullptr) {
		suffixes.push_back(culture->get_identifier());
		suffixes.push_back(culture->get_culture_group()->get_identifier());
	}

	return translator::get()->translate(landed_title::get_tier_identifier(this->get_tier()), suffixes);
}

/**
**	@brief	Get the landed title's titled name
**
**	@return	The landed title's titled name
*/
std::string landed_title::get_titled_name() const
{
	std::string titled_name = this->get_tier_title_name() + " of ";
	titled_name += this->get_name();
	return titled_name;
}

/**
**	@brief	Get the landed title's holder title name
**
**	@return	The landed title's holder title name
*/
std::string landed_title::get_holder_title_name() const
{
	const culture *culture = this->get_culture();

	std::vector<std::string> suffixes;

	if (this->get_holding() != nullptr) {
		//for non-titular baronies, use the holding's type for the localization, so that e.g. a city's holder title name will be "mayor", regardless of the character's actual government
		suffixes.push_back(this->get_holding()->get_type()->get_identifier());
	}

	if (culture != nullptr) {
		suffixes.push_back(culture->get_identifier());
		suffixes.push_back(culture->get_culture_group()->get_identifier());
	}

	return translator::get()->translate(landed_title::get_tier_holder_identifier(this->get_tier()), suffixes);
}

void landed_title::set_holder(character *character)
{
	if (character == this->get_holder()) {
		return;
	}

	if (this->get_holder() != nullptr) {
		this->get_holder()->remove_landed_title(this);
	}

	this->holder = character;

	if (character != nullptr) {
		character->add_landed_title(this);
	}
	this->holder_title = nullptr; //set the holder title to null, so that the new holder (null or otherwise) isn't overwritten by a previous holder title

	if (this->get_province() != nullptr) {
		//if this is a non-titular county, then the character holding it must also possess the county's capital holding
		this->get_province()->get_capital_holding()->get_barony()->set_holder(character);

		//if this is a non-titular county, the fort, university and hospital of its province must belong to the county holder
		holding *fort_holding = this->get_province()->get_fort_holding_slot()->get_holding();
		if (fort_holding != nullptr) {
			fort_holding->set_owner(character);
		}

		holding *university_holding = this->get_province()->get_university_holding_slot()->get_holding();
		if (university_holding != nullptr) {
			university_holding->set_owner(character);
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
	this->holder_title = title;
	this->holder = nullptr; //set the holder title to null, so that the new holder title isn't overwritten by a previous holder

}

void landed_title::set_de_jure_liege_title(landed_title *title)
{
	if (title == this->get_de_jure_liege_title()) {
		return;
	}

	if (this->get_de_jure_liege_title() != nullptr) {
		this->get_de_jure_liege_title()->remove_de_jure_vassal_title(this);
	}

	if (static_cast<int>(title->get_tier()) - static_cast<int>(this->get_tier()) != 1) {
		throw std::runtime_error("Tried to set title \"" + title->get_identifier() + "\" as the de jure liege of \"" + this->get_identifier() + "\", but the former is not one title tier above the latter.");
	}

	this->de_jure_liege_title = title;
	title->add_de_jure_vassal_title(this);

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

	for (int i = static_cast<int>(tier) - 1; i >= static_cast<int>(landed_title_tier::county); --i) {
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

	if (tier > landed_title_tier::county) {
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

/**
**	@brief	Get the title's de jure county
**
**	@return	The title's de jure county
*/
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

/**
**	@brief	Get the title's de jure duchy
**
**	@return	The title's de jure duchy
*/
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

/**
**	@brief	Get the title's de jure kingdom
**
**	@return	The title's de jure kingdom
*/
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

/**
**	@brief	Get the title's de jure empire
**
**	@return	The title's de jure empire
*/
landed_title *landed_title::get_de_jure_empire() const
{
	return this->get_tier_de_jure_title(landed_title_tier::empire);
}

/**
**	@brief	Get the title's culture
**
**	@return	The title's culture
*/
culture *landed_title::get_culture() const
{
	if (this->get_holder() != nullptr) {
		return this->get_holder()->get_culture();
	} else if (this->get_capital_province() != nullptr) {
		return this->get_capital_province()->get_culture();
	}

	return nullptr;
}

/**
**	@brief	Get the title's religion
**
**	@return	The title's religion
*/
religion *landed_title::get_religion() const
{
	if (this->get_holder() != nullptr) {
		return this->get_holder()->get_religion();
	} else if (this->get_capital_province() != nullptr) {
		return this->get_capital_province()->get_religion();
	}

	return nullptr;
}

/**
**	@brief	Get the path to the title's flag
**
**	@return	The path to the flag
*/
std::filesystem::path landed_title::get_flag_path() const
{
	std::string base_tag = this->get_flag_tag();
	std::vector<std::vector<std::string>> tag_list_with_fallbacks;

	culture *culture = this->get_culture();
	if (culture != nullptr) {
		tag_list_with_fallbacks.push_back({culture->get_identifier(), culture->get_culture_group()->get_identifier()});
	}

	religion *religion = this->get_religion();
	if (religion != nullptr) {
		tag_list_with_fallbacks.push_back({religion->get_identifier()});
	}

	std::filesystem::path flag_path = database::get_tagged_image_path(database::get_flags_path(), base_tag, tag_list_with_fallbacks, ".svg");
	return flag_path;
}

QVariantList landed_title::get_laws_qvariant_list() const
{
	return util::container_to_qvariant_list(this->get_laws());
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
	this->laws[law->get_group()] = law;
}

Q_INVOKABLE void landed_title::remove_law(law *law)
{
	if (this->has_law(law)) {
		this->laws.erase(law->get_group());
	}
}

}
