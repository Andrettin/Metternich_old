#pragma once

namespace metternich {

class gsml_data;
class gsml_property;

//a visitor that only does an action when visiting GSML data
template <typename function_type>
class gsml_data_visitor
{
public:
	gsml_data_visitor(const function_type &function) : function(function)
	{
	}

	gsml_data_visitor(function_type &&function) = delete;

	void operator()(const gsml_property &property) const
	{
		Q_UNUSED(property)
	}

	void operator()(const gsml_data &scope) const
	{
		this->function(scope);
	}

private:
	const function_type &function;
};

}
