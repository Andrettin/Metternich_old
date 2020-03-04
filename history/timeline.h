#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QDateTime>

#include <map>
#include <string>

namespace metternich {

class timeline final : public data_entry, public data_type<timeline>
{
	Q_OBJECT

	Q_PROPERTY(metternich::timeline* parent_timeline MEMBER parent_timeline READ get_parent_timeline)
	Q_PROPERTY(QDateTime point_of_divergence MEMBER point_of_divergence READ get_point_of_divergence)

public:
	static constexpr const char *class_identifier = "timeline";
	static constexpr const char *database_folder = "timelines";

	timeline(const std::string &identifier) : data_entry(identifier) {}

	timeline *get_parent_timeline() const
	{
		return this->parent_timeline;
	}

	const QDateTime &get_point_of_divergence() const
	{
		return this->point_of_divergence;
	}

	bool derives_from_timeline(const timeline *timeline) const
	{
		if (this->get_parent_timeline() == nullptr) {
			return false;
		}

		if (timeline == this->get_parent_timeline()) {
			return true;
		}

		return this->get_parent_timeline()->derives_from_timeline(timeline);
	}

	bool contains_timeline_date(const timeline *timeline, const QDateTime &date) const
	{
		if (timeline == this) {
			return true;
		} else if (this->get_parent_timeline() == nullptr) {
			return false;
		}

		if (timeline == this->get_parent_timeline()) {
			return date < point_of_divergence;
		}

		return this->get_parent_timeline()->contains_timeline_date(timeline, date);
	}

private:
	timeline *parent_timeline = nullptr; //the timeline from which this one derives (null means the default timeline)
	QDateTime point_of_divergence; //the point of divergence from the parent timeline
};

}
