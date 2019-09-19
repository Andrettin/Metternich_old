#include "culture/culture.h"

#include "character/dynasty.h"
#include "culture/culture_group.h"
#include "random.h"

namespace metternich {

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void culture::ProcessGSMLScope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for cultures needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->color.setRgb(red, green, blue);
	} else {
		culture_base::ProcessGSMLScope(scope);
	}
}

/**
**	@brief	Check whether the culture is in a valid state
*/
void culture::Check() const
{
	if (this->get_culture_group() == nullptr) {
		throw std::runtime_error("Culture \"" + this->GetIdentifier() + "\" has no culture group.");
	}

	if (!this->get_color().isValid()) {
		throw std::runtime_error("Culture \"" + this->GetIdentifier() + "\" has no valid color.");
	}

	if (this->get_default_phenotype() == nullptr && this->get_culture_group()->get_default_phenotype() == nullptr) {
		throw std::runtime_error("Culture \"" + this->GetIdentifier() + "\" has no default phenotype, and neither does its culture group.");
	}

	if (this->get_male_names().empty() && this->get_culture_group()->get_male_names().empty()) {
		throw std::runtime_error("Culture \"" + this->GetIdentifier() + "\" has no male names, and neither does its culture group (\"" + this->get_culture_group()->GetIdentifier() + "\").");
	}

	if (this->get_female_names().empty() && this->get_culture_group()->get_female_names().empty()) {
		throw std::runtime_error("Culture \"" + this->GetIdentifier() + "\" has no female names, and neither does its culture group (\"" + this->get_culture_group()->GetIdentifier() + "\").");
	}
}

/**
**	@brief	Generate male name
*/
std::string culture::generate_male_name() const
{
	if (!this->get_male_names().empty()) {
		return this->get_male_names()[Random::Generate(this->get_male_names().size())];
	}

	if (!this->get_culture_group()->get_male_names().empty()) {
		return this->get_culture_group()->get_male_names()[Random::Generate(this->get_culture_group()->get_male_names().size())];
	}

	return std::string();
}

/**
**	@brief	Generate female name
*/
std::string culture::generate_female_name() const
{
	if (!this->get_female_names().empty()) {
		return this->get_female_names()[Random::Generate(this->get_female_names().size())];
	}

	if (!this->get_culture_group()->get_female_names().empty()) {
		return this->get_culture_group()->get_female_names()[Random::Generate(this->get_culture_group()->get_female_names().size())];
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

	return this->dynasties[Random::Generate(this->dynasties.size())]->get_name();
}

}
