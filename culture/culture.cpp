#include "culture/culture.h"

#include "character/dynasty.h"
#include "culture/culture_group.h"
#include "random.h"
#include "util/container_util.h"

namespace metternich {

/**
**	@brief	Initialize the culture
*/
void culture::initialize()
{
	if (this->get_species() == nullptr) {
		this->set_species(this->get_culture_group()->get_species());
	}

	if (this->get_default_phenotype() == nullptr) {
		this->set_default_phenotype(this->get_culture_group()->get_default_phenotype());
	}

	for (const std::string &name : this->get_culture_group()->get_male_names()) {
		this->add_male_name(name);
	}

	for (const std::string &name : this->get_culture_group()->get_female_names()) {
		this->add_female_name(name);
	}

	this->male_name_generation_list = util::container_to_vector(this->get_male_names());
	this->female_name_generation_list = util::container_to_vector(this->get_female_names());
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

	if (this->get_default_phenotype() == nullptr) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no default phenotype.");
	}

	if (this->get_male_names().empty()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no male names.");
	}

	if (this->get_female_names().empty()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no female names.");
	}

	culture_base::check();
}

/**
**	@brief	Generate male name
*/
std::string culture::generate_male_name() const
{
	if (!this->male_name_generation_list.empty()) {
		return this->male_name_generation_list[random::generate(this->male_name_generation_list.size())];
	}

	return std::string();
}

/**
**	@brief	Generate female name
*/
std::string culture::generate_female_name() const
{
	if (!this->female_name_generation_list.empty()) {
		return this->female_name_generation_list[random::generate(this->female_name_generation_list.size())];
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
