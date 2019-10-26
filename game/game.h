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
enum class GameSpeed : int;

/**
**	@brief	The game instance
*/
class Game : public QObject, public singleton<Game>
{
	Q_OBJECT

	Q_PROPERTY(bool running READ IsRunning NOTIFY RunningChanged)
	Q_PROPERTY(QDateTime current_date READ GetCurrentDate NOTIFY CurrentDateChanged)
	Q_PROPERTY(QString current_date_string READ GetCurrentDateString NOTIFY CurrentDateChanged)
	Q_PROPERTY(metternich::character* player_character READ get_player_character NOTIFY player_character_changed)

public:
	Game();

	void Start(const QDateTime &start_date);
	void Run();
	void DoTick();
	void DoDay();
	void DoMonth();
	void DoYear();

	bool IsStarting() const
	{
		return this->Starting;
	}

	bool IsRunning() const
	{
		return this->Running;
	}

	const QDateTime &GetCurrentDate() const
	{
		return this->CurrentDate;
	}

	QString GetCurrentDateString() const
	{
		QLocale english_locale(QLocale::English);
		return english_locale.toString(this->CurrentDate, "d MMMM, yyyy");
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

	void PostOrder(const std::function<void()> &function)
	{
		std::unique_lock<std::shared_mutex> lock(this->Mutex);
		this->Orders.push(function);
	}

	void DoOrders()
	{
		std::shared_lock<std::shared_mutex> lock(this->Mutex);

		while (!this->Orders.empty()) {
			this->Orders.front()();
			this->Orders.pop();
		}
	}

	void generate_missing_title_holders();
	void purge_superfluous_characters();

signals:
	void RunningChanged();
	void CurrentDateChanged();
	void player_character_changed();

private:
	bool Starting = false;
	bool Running = false;
	QDateTime CurrentDate;
	GameSpeed Speed;
	character *player_character = nullptr;
	std::queue<std::function<void()>> Orders; //orders given by the player, received from the UI thread
	mutable std::shared_mutex Mutex;
};

}
