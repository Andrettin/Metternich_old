#pragma once

#include "util/singleton.h"

#include <QDateTime>

#include <filesystem>
#include <string>

namespace metternich {

class gsml_property;
class landed_title;
class law_group;
class timeline;
class world;
enum class game_speed;

class defines final : public QObject, public singleton<defines>
{
	Q_OBJECT

	Q_PROPERTY(metternich::timeline* default_timeline MEMBER default_timeline READ get_default_timeline)
	Q_PROPERTY(QDateTime start_date MEMBER start_date READ get_start_date)
	Q_PROPERTY(metternich::landed_title* player_character_title MEMBER player_character_title READ get_player_character_title)
	Q_PROPERTY(int base_population_growth MEMBER base_population_growth READ get_base_population_growth)
	Q_PROPERTY(int cultural_derivation_factor MEMBER cultural_derivation_factor READ get_cultural_derivation_factor)
	Q_PROPERTY(int max_settlement_slots_per_province MEMBER max_settlement_slots_per_province READ get_max_settlement_slots_per_province)
	Q_PROPERTY(int max_palace_slots_per_province MEMBER max_palace_slots_per_province READ get_max_palace_slots_per_province)
	Q_PROPERTY(metternich::law_group* succession_law_group MEMBER succession_law_group READ get_succession_law_group)
	Q_PROPERTY(int trade_node_score_realm_modifier MEMBER trade_node_score_realm_modifier READ get_trade_node_score_realm_modifier)
	Q_PROPERTY(int trade_node_score_culture_modifier MEMBER trade_node_score_culture_modifier READ get_trade_node_score_culture_modifier)
	Q_PROPERTY(int trade_node_score_culture_group_modifier MEMBER trade_node_score_culture_group_modifier READ get_trade_node_score_culture_group_modifier)
	Q_PROPERTY(int trade_node_score_religion_modifier MEMBER trade_node_score_religion_modifier READ get_trade_node_score_religion_modifier)
	Q_PROPERTY(int trade_node_score_religion_group_modifier MEMBER trade_node_score_religion_group_modifier READ get_trade_node_score_religion_group_modifier)
	Q_PROPERTY(int trade_cost_modifier_per_distance MEMBER trade_cost_modifier_per_distance READ get_trade_cost_modifier_per_distance)
	Q_PROPERTY(int base_port_trade_cost_modifier MEMBER base_port_trade_cost_modifier READ get_base_port_trade_cost_modifier)

public:

	defines();

	void load(const std::filesystem::path &base_path);
	void process_gsml_property(const gsml_property &property);

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

	law_group *get_succession_law_group() const
	{
		return this->succession_law_group;
	}

	int get_trade_node_score_realm_modifier() const
	{
		return trade_node_score_realm_modifier;
	}

	int get_trade_node_score_culture_modifier() const
	{
		return trade_node_score_culture_modifier;
	}

	int get_trade_node_score_culture_group_modifier() const
	{
		return trade_node_score_culture_group_modifier;
	}

	int get_trade_node_score_religion_modifier() const
	{
		return trade_node_score_religion_modifier;
	}

	int get_trade_node_score_religion_group_modifier() const
	{
		return trade_node_score_religion_group_modifier;
	}

	int get_trade_cost_modifier_per_distance() const
	{
		return trade_cost_modifier_per_distance;
	}

	int get_base_port_trade_cost_modifier() const
	{
		return base_port_trade_cost_modifier;
	}

private:
	timeline *default_timeline = nullptr;
	QDateTime start_date;
	landed_title *player_character_title = nullptr;
	int base_population_growth = 0; //permyriad
	game_speed default_game_speed;
	int cultural_derivation_factor = 1; //permyriad; up to this fraction of people in a population unit will change cultures due to cultural derivation per month per derived culture
	int max_settlement_slots_per_province = 0;
	int max_palace_slots_per_province = 0;
	law_group *succession_law_group = nullptr;
	int trade_node_score_realm_modifier = 0;
	int trade_node_score_culture_modifier = 0;
	int trade_node_score_culture_group_modifier = 0;
	int trade_node_score_religion_modifier = 0;
	int trade_node_score_religion_group_modifier = 0;
	int trade_cost_modifier_per_distance = 0; //trade cost modifier for every [province::base_distance] kilometers
	int base_port_trade_cost_modifier = 0; //base trade cost modifier when going from a water province to a land one or vice-versa
};

}
