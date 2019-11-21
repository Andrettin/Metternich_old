#include "culture/culture.h"

#include "character/dynasty.h"
#include "culture/culture_group.h"
#include "random.h"

namespace metternich {

/**
**	@brief	Initialize the culture
*/
void culture::initialize()
{
	if (this->get_species() == nullptr) {
		this->set_species(this->get_culture_group()->get_species());
	}
}

/**
**	@brief	Check whether the culture is in a valid state
*/
void culture::check() const
{
	if (this->get_culture_group() == nullptr) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no culture group.");
	}

	if (this->get_species() == nullptr) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no species.");
	}

	if (!this->get_color().isValid()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no valid color.");
	}

	if (this->get_default_phenotype() == nullptr && this->get_culture_group()->get_default_phenotype() == nullptr) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no default phenotype, and neither does its culture group.");
	}

	if (this->get_male_names().empty() && this->get_culture_group()->get_male_names().empty()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no male names, and neither does its culture group (\"" + this->get_culture_group()->get_identifier() + "\").");
	}

	if (this->get_female_names().empty() && this->get_culture_group()->get_female_names().empty()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no female names, and neither does its culture group (\"" + this->get_culture_group()->get_identifier() + "\").");
	}

	culture_base::check();
}

/**
**	@brief	Generate male name
*/
std::string culture::generate_male_name() const
{
	if (!this->get_male_names().empty()) {
		return this->get_male_names()[random::generate(this->get_male_names().size())];
	}

	if (!this->get_culture_group()->get_male_names().empty()) {
		return this->get_culture_group()->get_male_names()[random::generate(this->get_culture_group()->get_male_names().size())];
	}

	return std::string();
}

/**
**	@brief	Generate female name
*/
std::string culture::generate_female_name() const
{
	if (!this->get_female_names().empty()) {
		return this->get_female_names()[random::generate(this->get_female_names().size())];
	}

	if (!this->get_culture_group()->get_female_names().empty()) {
		return this->get_culture_group()->get_female_names()[random::generate(this->get_culture_group()->get_female_names().size())];
	}

	return std::string();
}

/**
**	@brief	Generate dynasty name
*/
std::string culture::generate_dynasty_name() const
{
	if (this->dynasties.empty()) {
		return std::string();
	}

	return this->dynasties[random::generate(this->dynasties.size())]->get_name();
}

}
