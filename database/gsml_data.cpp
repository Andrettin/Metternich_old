#include "database/gsml_data.h"

#include "database/gsml_operator.h"

namespace metternich {

gsml_data::gsml_data(std::string &&tag)
	: tag(std::move(tag)), scope_operator(gsml_operator::assignment)
{
}

void gsml_data::add_property(const std::string &key, const std::string &value)
{
	this->properties.emplace_back(key, gsml_operator::assignment, value);
}

void gsml_data::print(std::ofstream &ofstream, const size_t indentation, const bool new_line) const
{
	if (new_line) {
		ofstream << std::string(indentation, '\t');
	} else {
		ofstream << " ";
	}
	if (!this->get_tag().empty()) {
		ofstream << this->get_tag() << " ";
		switch (this->get_operator()) {
			case gsml_operator::assignment:
				ofstream << "=";
				break;
			case gsml_operator::addition:
				ofstream << "+=";
				break;
			case gsml_operator::subtraction:
				ofstream << "-=";
				break;
			case gsml_operator::none:
				throw std::runtime_error("Cannot print the GSML \"none\" operator.");
		}
	}
	ofstream << "{";
	if (!this->is_minor()) {
		ofstream << "\n";
	}

	this->print_components(ofstream, indentation + 1);

	if (!this->is_minor()) {
		ofstream << std::string(indentation, '\t');
	}
	ofstream << "}";
	if (!this->is_minor()) {
		ofstream << "\n";
	}
}

}
