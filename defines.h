#pragma once

#include "singleton.h"

#include <QDateTime>

#include <string>

namespace metternich {

class gsml_property;

/**
**	@brief	Defines for the engine which are loaded from the database
*/
class Defines : public QObject, public singleton<Defines>
{
	Q_OBJECT

	Q_PROPERTY(QDateTime start_date MEMBER StartDate READ GetStartDate)
	Q_PROPERTY(int player_character MEMBER player_character_id READ get_player_character_id)
	Q_PROPERTY(int base_population_growth MEMBER BasePopulationGrowth READ GetBasePopulationGrowth)

public:
	void load();
	void process_gsml_property(const gsml_property &property);

	const QDateTime &GetStartDate() const
	{
		return this->StartDate;
	}

	int get_player_character_id() const
	{
		return this->player_character_id;
	}

	int GetBasePopulationGrowth() const
	{
		return this->BasePopulationGrowth;
	}

private:
	QDateTime StartDate;
	int player_character_id = 0;
	int BasePopulationGrowth = 0; //permyriad
};

}
