#include "culture/culture_base.h"

#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "util/vector_util.h"

namespace metternich {

void culture_base::process_gsml_scope(const gsml_data &scope)
{
	const std::string &tag = scope.get_tag();
	const std::vector<std::string> &values = scope.get_values();

	if (tag == "male_names") {
		switch (scope.get_operator()) {
			case gsml_operator::assignment:
				this->male_names = values;
				break;
			case gsml_operator::addition:
				vector::merge(this->male_names, values);
				break;
			default:
				throw std::runtime_error("Invalid operator for scope (\"" + tag + "\").");
		}
	} else if (tag == "female_names") {
		switch (scope.get_operator()) {
			case gsml_operator::assignment:
				this->female_names = values;
				break;
			case gsml_operator::addition:
				vector::merge(this->female_names, values);
				break;
			default:
				throw std::runtime_error("Invalid operator for scope (\"" + tag + "\").");
		}
	} else if (tag == "dynasty_names") {
		switch (scope.get_operator()) {
			case gsml_operator::assignment:
				this->dynasty_names = values;
				break;
			case gsml_operator::addition:
				vector::merge(this->dynasty_names, values);
				break;
			default:
				throw std::runtime_error("Invalid operator for scope (\"" + tag + "\").");
		}
	} else {
		data_entry_base::process_gsml_scope(scope);
	}
}

void culture_base::check() const
{
	if (!this->get_color().isValid()) {
		throw std::runtime_error("Culture \"" + this->get_identifier() + "\" has no valid color.");
	}
}

}
