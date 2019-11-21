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

/**
**	@brief	The game instance
*/
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

	const QDateTime &get_current_date() const
	{
		return this->current_date;
	}

	QString get_current_date_string() const
	{
		QLocale english_locale(QLocale::English);
		return english_locale.toString(this->current_date, "d MMMM, yyyy");
	}

	character *get_player_character() const
	{
		return this->player_character;
	}

	void set_player_character(character *character)
	{
		if (this->player_character == character) {
			return;
		}

		this->player_character = character;
		emit player_character_changed();
	}

	void post_order(const std::function<void()> &function)
	{
		std::unique_lock<std::shared_mutex> lock(this->mutex);
		this->orders.push(function);
	}

	void do_orders()
	{
		std::shared_lock<std::shared_mutex> lock(this->mutex);

		while (!this->orders.empty()) {
			this->orders.front()();
			this->orders.pop();
		}
	}

	void generate_missing_title_holders();
	void purge_superfluous_characters();

signals:
	void running_changed();
	void current_date_changed();
	void player_character_changed();

private:
	bool starting = false;
	bool running = false;
	std::atomic<bool> should_stop = false;
	QDateTime current_date;
	game_speed speed;
	character *player_character = nullptr;
	std::queue<std::function<void()>> orders; //orders given by the player, received from the UI thread
	mutable std::shared_mutex mutex;
};

}
