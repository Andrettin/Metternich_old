#pragma once

#include <string>

namespace Metternich {

class Building
{
public:
	static constexpr const char *ClassIdentifier = "building";

	const std::string &GetName() const
	{
		return this->Name;
	}

private:
	std::string Name;
};

}
