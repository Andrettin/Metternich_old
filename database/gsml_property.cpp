#include "database/gsml_property.h"

#include "database/gsml_operator.h"

namespace metternich {

void gsml_property::print(std::ofstream &ofstream, const size_t indentation) const
{
	ofstream << std::string(indentation, '\t') << this->get_key() << " ";

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

	ofstream << " " << this->get_value() << "\n";
}

}
