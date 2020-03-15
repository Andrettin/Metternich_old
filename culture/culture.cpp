#include "culture/culture.h"

#include "character/dynasty.h"
#include "culture/culture_group.h"
#include "culture/culture_supergroup.h"
#include "script/condition/and_condition.h"
#include "util/container_util.h"
#include "util/random.h"
#include "util/vector_random_util.h"

namespace metternich {

culture::culture(const std::string &identifier) : culture_base(identifier)
{
}

culture::~culture()
{
}

void culture::process_gsml_scope(const gsml_data &scope)
{
	if (scope.get_tag() == "derivation_conditions") {
		this->derivation_conditions = std::make_unique<and_condition<population_unit>>();
		database::process_gsml_data(this->derivation_conditions.get(), scope);
	} else {
		culture_base::process_gsml_scope(scope);
	}
}

void culture::initialize()
{
	if (this->get_group() == nullptr) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no culture group.");
	}

	if (!this->get_group()->is_initialized()) {
		this->get_group()->initialize();
	}

	if (this->get_species() == nullptr) {
		this->set_species(this->get_group()->get_species());
	}

	if (this->get_default_phenotype() == nullptr) {
		this->set_default_phenotype(this->get_group()->get_default_phenotype());
	}

	if (!this->get_male_names().empty()) {
		this->get_group()->add_male_names(this->get_male_names());
		this->get_supergroup()->add_male_names(this->get_male_names());
	}

	if (!this->get_female_names().empty()) {
		this->get_group()->add_female_names(this->get_female_names());
		this->get_supergroup()->add_female_names(this->get_male_names());
	}

	if (!this->get_dynasty_names().empty()) {
		this->get_group()->add_dynasty_names(this->get_dynasty_names());
		this->get_supergroup()->add_dynasty_names(this->get_male_names());
	}

	data_entry_base::initialize();
}

void culture::check() const
{
	if (this->get_species() == nullptr) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no species.");
	}

	if (!this->get_color().isValid()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no valid color.");
	}

	if (this->get_default_phenotype() == nullptr) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no default phenotype.");
	}

	if (this->get_male_names().empty() && this->get_group()->get_male_names().empty() && this->get_supergroup()->get_male_names().empty()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no male names, and nor does its culture group or supergroup.");
	}

	if (this->get_female_names().empty() && this->get_group()->get_female_names().empty() && this->get_supergroup()->get_female_names().empty()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no female names, and nor does its culture group or supergroup.");
	}

	if (this->get_dynasty_names().empty() && this->get_group()->get_dynasty_names().empty() && this->get_supergroup()->get_dynasty_names().empty()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no dynasty names, and nor does its culture group or supergroup.");
	}

	culture_base::check();
}

culture_supergroup *culture::get_supergroup() const
{
	if (this->get_group() != nullptr) {
		return this->get_group()->get_supergroup();
	}

	return nullptr;
}

QVariantList culture::get_derived_cultures_qvariant_list() const
{
	return container::to_qvariant_list(this->get_derived_cultures());
}

std::string culture::generate_male_name() const
{
	if (!this->get_male_names().empty()) {
		return vector::get_random(this->get_male_names());
	}

	if (!this->get_group()->get_male_names().empty()) {
		return vector::get_random(this->get_group()->get_male_names());
	}

	if (!this->get_supergroup()->get_male_names().empty()) {
		return vector::get_random(this->get_supergroup()->get_male_names());
	}

	throw std::runtime_error("No male name could be generated for culture \"" + this->get_identifier() + "\"");
}

std::string culture::generate_female_name() const
{
	if (!this->get_female_names().empty()) {
		return vector::get_random(this->get_female_names());
	}

	if (!this->get_group()->get_female_names().empty()) {
		return vector::get_random(this->get_group()->get_female_names());
	}

	if (!this->get_supergroup()->get_female_names().empty()) {
		return vector::get_random(this->get_supergroup()->get_female_names());
	}

	throw std::runtime_error("No female name could be generated for culture \"" + this->get_identifier() + "\"");
}

std::string culture::generate_dynasty_name() const
{
	if (!this->get_dynasty_names().empty()) {
		return vector::get_random(this->get_dynasty_names());
	}

	if (!this->get_group()->get_dynasty_names().empty()) {
		return vector::get_random(this->get_group()->get_dynasty_names());
	}

	if (!this->get_supergroup()->get_dynasty_names().empty()) {
		return vector::get_random(this->get_supergroup()->get_dynasty_names());
	}

	throw std::runtime_error("No dynasty name could be generated for culture \"" + this->get_identifier() + "\"");
}

}
