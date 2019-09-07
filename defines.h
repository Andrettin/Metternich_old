#pragma once

#include "singleton.h"

#include <QDateTime>

#include <string>

namespace metternich {

class gsml_property;

/**
**	@brief	Defines for the engine which are loaded from the database
*/
class Defines : public QObject, public Singleton<Defines>
{
	Q_OBJECT

	Q_PROPERTY(QDateTime start_date MEMBER StartDate READ GetStartDate)
	Q_PROPERTY(int player_character MEMBER PlayerCharacterID READ GetPlayerCharacterID)
	Q_PROPERTY(int base_population_growth MEMBER BasePopulationGrowth READ GetBasePopulationGrowth)

public:
	void Load();
	void ProcessGSMLProperty(const gsml_property &property);

	const QDateTime &GetStartDate() const
	{
		return this->StartDate;
	}

	int GetPlayerCharacterID() const
	{
		return this->PlayerCharacterID;
	}

	int GetBasePopulationGrowth() const
	{
		return this->BasePopulationGrowth;
	}

private:
	QDateTime StartDate;
	int PlayerCharacterID = 0;
	int BasePopulationGrowth = 0; //permyriad
};

}
