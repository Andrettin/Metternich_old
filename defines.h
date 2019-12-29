#pragma once

#include "singleton.h"

#include <QDateTime>

#include <filesystem>
#include <string>

namespace metternich {

class gsml_property;
class landed_title;
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
	Q_PROPERTY(metternich::landed_title* player_character_title MEMBER player_character_title READ get_player_character_title)
	Q_PROPERTY(int base_population_growth MEMBER base_population_growth READ get_base_population_growth)
	Q_PROPERTY(int cultural_derivation_factor MEMBER cultural_derivation_factor READ get_cultural_derivation_factor)
	Q_PROPERTY(int max_settlement_slots_per_province MEMBER max_settlement_slots_per_province READ get_max_settlement_slots_per_province)
	Q_PROPERTY(int max_palace_slots_per_province MEMBER max_palace_slots_per_province READ get_max_palace_slots_per_province)

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

	landed_title *get_player_character_title() const
	{
		return this->player_character_title;
	}

	int get_base_population_growth() const
	{
		return this->base_population_growth;
	}

	game_speed get_default_game_speed() const
	{
		return this->default_game_speed;
	}

	int get_cultural_derivation_factor() const
	{
		return this->cultural_derivation_factor;
	}

	int get_max_settlement_slots_per_province() const
	{
		return this->max_settlement_slots_per_province;
	}

	int get_max_palace_slots_per_province() const
	{
		return this->max_palace_slots_per_province;
	}

private:
	world *default_world = nullptr;
	timeline *default_timeline = nullptr;
	QDateTime start_date;
	landed_title *player_character_title = nullptr;
	int base_population_growth = 0; //permyriad
	game_speed default_game_speed;
	int cultural_derivation_factor = 1; //permyriad; up to this fraction of people in a population unit will change cultures due to cultural derivation per month per derived culture
	int max_settlement_slots_per_province = 0;
	int max_palace_slots_per_province = 0;
};

}
