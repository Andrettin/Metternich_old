#pragma once

#include <QDateTime>
#include <QLocale>
#include <QObject>

#include <memory>
#include <mutex>
#include <thread>

namespace Metternich {

enum class GameSpeed : int;

/**
**	@brief	The game instance
*/
class Game : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QDateTime current_date READ GetCurrentDate NOTIFY CurrentDateChanged)
	Q_PROPERTY(QString current_date_string READ GetCurrentDateString NOTIFY CurrentDateChanged)

public:
	static Game *GetInstance();

private:
	static inline std::unique_ptr<Game> Instance;
	static inline std::once_flag OnceFlag;

public:
	Game();

	void Start(const QDateTime &start_date);
	void Run();

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

signals:
	void RunningChanged();
	void CurrentDateChanged();

private:
	bool Running = false;
	QDateTime CurrentDate;
	GameSpeed Speed;
};

}
