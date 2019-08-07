#pragma once

#include <QDateTime>

#include <string>

namespace Metternich {

class GSMLProperty;

/**
**	@brief	Defines for the engine which are loaded from the database
*/
class Defines
{
public:
	static void Load();
	static bool ProcessGSMLProperty(const GSMLProperty &property);

	static const QDateTime &GetStartDate()
	{
		return Defines::StartDate;
	}

	static int GetPlayerCharacterID()
	{
		return Defines::PlayerCharacterID;
	}

private:
	static inline QDateTime StartDate;
	static inline int PlayerCharacterID = 0;
};

}
