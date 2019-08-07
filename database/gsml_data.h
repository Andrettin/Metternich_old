#pragma once

#include "database/gsml_property.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

namespace Metternich {

/**
**	@brief	Grand strategy markup language data
*/
class GSMLData
{
public:
	GSMLData(const std::string tag) : Tag(tag) {}

	static GSMLData ParseFile(const std::filesystem::path &filepath);

private:
	static std::vector<std::string> ParseLine(const std::string &line);
	static bool ParseEscapedCharacter(std::string &current_string, const char c);
	static void ParseTokens(const std::vector<std::string> &tokens, GSMLData **current_gsml_data);

public:
	const std::string &GetTag() const
	{
		return this->Tag;
	}

	const GSMLData *GetParent() const
	{
		return this->Parent;
	}

	const std::vector<GSMLData> &GetChildren() const
	{
		return this->Children;
	}

	const std::vector<GSMLProperty> &GetProperties() const
	{
		return this->Properties;
	}

	const std::vector<std::string> &GetValues() const
	{
		return this->Values;
	}

	void SortChildren()
	{
		//sort children by tag, alphabetically
		std::sort(this->Children.begin(), this->Children.end(), [](GSMLData &a, GSMLData &b) {
			return a.GetTag() < b.GetTag();
		});
	}

private:
	std::string Tag;
	GSMLData *Parent = nullptr;
	std::vector<GSMLData> Children;
	std::vector<GSMLProperty> Properties;
	std::vector<std::string> Values; //values directly attached to the GSML data scope, used for e.g. name arrays
};

}
