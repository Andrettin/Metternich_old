#pragma once

#include "singleton.h"

#include <QDateTime>
#include <QString>

#include <string>
#include <vector>

namespace metternich {

class timeline;

class history : public singleton<history>
{
public:
	static void generate_population_units();

	static QDateTime string_to_date(const std::string &date_str);

public:
	void load();

	bool is_loading() const
	{
		return this->loading;
	}

	const timeline *get_timeline() const
	{
		return this->timeline;
	}

	void set_timeline(const timeline *timeline)
	{
		this->timeline = timeline;
	}

	const QDateTime &get_start_date() const
	{
		return this->start_date;
	}

	void set_start_date(const QDateTime &date)
	{
		this->start_date = date;
	}

	bool contains_timeline_date(const timeline *timeline, const QDateTime &date) const;

private:
	bool loading = false;
	const timeline *timeline = nullptr;
	QDateTime start_date;
};

}
