#pragma once

#include "singleton.h"

#include <QDateTime>

#include <filesystem>
#include <string>

namespace metternich {

class gsml_property;
class timeline;
class world;
enum class game_speed : int;

/**
**	@brief	Defines for the engine which are loaded from the database
*/
class defines : public QObject, public singleton<defines>
{
	Q_OBJECT

	Q_PROPERTY(metternich::world* default_world MEMBER default_world READ get_default_world)
	Q_PROPERTY(metternich::timeline* default_timeline MEMBER default_timeline READ get_default_timeline)
	Q_PROPERTY(QDateTime start_date MEMBER start_date READ get_start_date)
	Q_PROPERTY(int player_character MEMBER player_character_id READ get_player_character_id)
	Q_PROPERTY(int base_population_growth MEMBER base_population_growth READ get_base_population_growth)

public:
	defines();

	void load(const std::filesystem::path &base_path);
	void process_gsml_property(const gsml_property &property);

	world *get_default_world() const
	{
		return this->default_world;
	}

	timeline *get_default_timeline() const
	{
		return this->default_timeline;
	}

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

	game_speed get_default_game_speed() const
	{
		return this->default_game_speed;
	}

private:
	world *default_world = nullptr;
	timeline *default_timeline = nullptr;
	QDateTime start_date;
	int player_character_id = 0;
	int base_population_growth = 0; //permyriad
	game_speed default_game_speed;
};

}
