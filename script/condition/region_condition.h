#pragma once

#include "map/province.h"
#include "map/region.h"
#include "script/condition/condition.h"

namespace metternich {

class region;

/**
**	@brief	A scripted region condition
*/
template <typename T>
class region_condition : public condition<T>
{
public:
	region_condition(const std::string &region_identifier)
	{
		this->region = region::get(region_identifier);
	}

	virtual const std::string &get_identifier() const override
	{
		static std::string identifier = "region";
		return identifier;
	}

	virtual bool check(const T *scope) const override
	{
		const province *province = nullptr;
		if constexpr (std::is_same_v<T, metternich::province>) {
			province = scope;
		} else {
			province = scope->get_province();
		}
		return province->is_in_region(this->region);
	}

private:
	region *region = nullptr;
};

}
