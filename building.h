#include <string>

class Building
{
public:
	const std::string &GetName() const
	{
		return this->Name;
	}

private:
	std::string Name;
};
