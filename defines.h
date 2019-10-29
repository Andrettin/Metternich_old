#pragma once

#include "singleton.h"

#include <QDateTime>

#include <string>

namespace metternich {

class gsml_property;

/**
**	@brief	Defines for the engine which are loaded from the database
*/
class defines : public QObject, public singleton<defines>
{
	Q_OBJECT

	Q_PROPERTY(QDateTime start_date MEMBER start_date READ get_start_date)
	Q_PROPERTY(int player_character MEMBER player_character_id READ get_player_character_id)
	Q_PROPERTY(int base_population_growth MEMBER base_population_growth READ get_base_population_growth)

public:
	void load();
	void process_gsml_property(const gsml_property &property);

	const QDateTime &get_start_date() const
	{
		return this->start_date;
	}

	int get_player_character_id() const
	{
		return this->player_character_id;
	}

	int get_base_population_growth() const
	{
		return this->base_population_growth;
	}

private:
	QDateTime start_date;
	int player_character_id = 0;
	int base_population_growth = 0; //permyriad
};

}
