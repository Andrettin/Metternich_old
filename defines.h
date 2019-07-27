#pragma once

#include <string>

class GSMLProperty;

/**
**	@brief	Defines for the engine which are loaded from the database
*/
class Defines
{
public:
	static void Load();
	static bool ProcessGSMLProperty(const GSMLProperty &property);

	static const std::string &GetAssetImportPath()
	{
		return Defines::AssetImportPath;
	}

private:
	static inline std::string AssetImportPath;
};
