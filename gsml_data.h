#pragma once

#include "gsml_property.h"

#include <filesystem>
#include <string>
#include <vector>

/**
**	@brief	Grand strategy markup language data
*/
class GSMLData
{
public:
	GSMLData(const std::string tag = std::string()) : Tag(tag) {}

	static GSMLData ParseFile(const std::filesystem::path &filepath);

private:
	static std::vector<std::string> ParseLine(const std::string &line);
	static bool ParseEscapedCharacter(std::string &current_string, const char c);
	static void ParseTokens(const std::vector<std::string> &tokens, GSMLData **current_gsml_data);

	std::string Tag;
	GSMLData *Parent = nullptr;
	std::vector<GSMLData> Children;
	std::vector<GSMLProperty> Properties;
};
