#pragma once

#include "singleton.h"

#include <QDateTime>
#include <QLocale>
#include <QObject>

#include <functional>
#include <queue>
#include <shared_mutex>
#include <thread>

namespace metternich {

class character;
class timeline;
enum class game_speed : int;
enum class tick_period : int;

class game : public QObject, public singleton<game>
{
	Q_OBJECT

	Q_PROPERTY(bool running READ is_running NOTIFY running_changed)
	Q_PROPERTY(QDateTime current_date READ get_current_date NOTIFY current_date_changed)
	Q_PROPERTY(QString current_date_string READ get_current_date_string NOTIFY current_date_changed)
	Q_PROPERTY(metternich::character* player_character READ get_player_character NOTIFY player_character_changed)

public:
	game();

	void start(const timeline *timeline, const QDateTime &start_date);

	void stop() {
		this->should_stop = true;
	}

	void run();
	void do_tick();
	void do_day();
	void do_month();
	void do_year();

	bool is_starting() const
	{
		return this->starting;
	}

	bool is_running() const
	{
		return this->running;
	}

	bool is_paused() const
	{
		return this->paused;
	}

	void set_paused(const bool paused)
	{
		if (paused == this->is_paused()) {
			return;
		}

		this->paused = paused;
		emit paused_changed();
	}

	void set_tick_period(const tick_period tick_period)
	{
		this->tick_period = tick_period;
	}

	const QDateTime &get_current_date() const
	{
		return this->current_date;
	}

	QString get_current_date_string() const;

	character *get_player_character() const
	{
		return this->player_character;
	}

	void set_player_character(character *character);

	void post_order(const std::function<void()> &function)
	{
		std::unique_lock<std::shared_mutex> lock(this->mutex);
		this->orders.push(function);
	}

	void do_orders()
	{
		std::function<void()> order;
		while ((order = this->get_next_order())) {
			order();
		}
	}

private:
	std::function<void()> get_next_order()
	{
		std::shared_lock<std::shared_mutex> lock(this->mutex);

		if (this->orders.empty()) {
			return std::function<void()>();
		}

		std::function<void()> order = std::move(this->orders.front());
		this->orders.pop();
		return order;
	}

	void generate_missing_title_holders();
	void purge_superfluous_characters();
	void amalgamate_map_inactive_worlds();

signals:
	void running_changed();
	void paused_changed();
	void current_date_changed();
	void player_character_changed();
	void tick_period_changed();

private:
	bool starting = false;
	bool running = false;
	bool paused = false;
	std::atomic<bool> should_stop = false;
	QDateTime current_date;
	game_speed speed;
	character *player_character = nullptr;
	unsigned long long total_ticks = 0; //the total amount of ticks which have passed in the game
	tick_period tick_period;
	std::queue<std::function<void()>> orders; //orders given by the player, received from the UI thread
	mutable std::shared_mutex mutex;
};

}
