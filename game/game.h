#pragma once

#include "singleton.h"

#include <QDateTime>
#include <QLocale>
#include <QObject>

#include <functional>
#include <queue>
#include <shared_mutex>
#include <thread>

namespace Metternich {

class Character;
enum class GameSpeed : int;

/**
**	@brief	The game instance
*/
class Game : public QObject, public Singleton<Game>
{
	Q_OBJECT

	Q_PROPERTY(bool running READ IsRunning NOTIFY RunningChanged)
	Q_PROPERTY(QDateTime current_date READ GetCurrentDate NOTIFY CurrentDateChanged)
	Q_PROPERTY(QString current_date_string READ GetCurrentDateString NOTIFY CurrentDateChanged)
	Q_PROPERTY(Metternich::Character* player_character READ GetPlayerCharacter NOTIFY PlayerCharacterChanged)

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

	Character *GetPlayerCharacter() const
	{
		return this->PlayerCharacter;
	}

	void SetPlayerCharacter(Character *character)
	{
		if (this->PlayerCharacter == character) {
			return;
		}

		this->PlayerCharacter = character;
		emit PlayerCharacterChanged();
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

	void GenerateMissingTitleHolders();
	void PurgeSuperfluousCharacters();

signals:
	void RunningChanged();
	void CurrentDateChanged();
	void PlayerCharacterChanged();

private:
	bool Starting = false;
	bool Running = false;
	QDateTime CurrentDate;
	GameSpeed Speed;
	Character *PlayerCharacter = nullptr;
	std::queue<std::function<void()>> Orders; //orders given by the player, received from the UI thread
	mutable std::shared_mutex Mutex;
};

}
