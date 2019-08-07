#pragma once

#include <QDateTime>
#include <QLocale>
#include <QObject>

#include <memory>
#include <mutex>
#include <thread>

namespace Metternich {

class Character;
enum class GameSpeed : int;

/**
**	@brief	The game instance
*/
class Game : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QDateTime current_date READ GetCurrentDate NOTIFY CurrentDateChanged)
	Q_PROPERTY(QString current_date_string READ GetCurrentDateString NOTIFY CurrentDateChanged)
	Q_PROPERTY(Metternich::Character* player_character READ GetPlayerCharacter NOTIFY PlayerCharacterChanged)

public:
	static Game *GetInstance();

private:
	static inline std::unique_ptr<Game> Instance;
	static inline std::once_flag OnceFlag;

public:
	Game();

	void Start(const QDateTime &start_date);
	void Run();

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
};

}
