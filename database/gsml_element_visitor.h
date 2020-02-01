#pragma once

namespace metternich {

class gsml_data;
class gsml_property;

template <typename property_function_type, typename data_function_type>
class gsml_element_visitor
{
public:
	gsml_element_visitor(const property_function_type &property_function, const data_function_type &data_function)
		: property_function(property_function), data_function(data_function)
	{
	}

	gsml_element_visitor(property_function_type &&property_function, data_function_type &&data_function) = delete;

	void operator()(const gsml_property &property) const
	{
		this->property_function(property);
	}

	void operator()(const gsml_data &scope) const
	{
		this->data_function(scope);
	}

private:
	const property_function_type &property_function;
	const data_function_type &data_function;
};

}
