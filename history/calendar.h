#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QDateTime>

#include <map>
#include <string>

namespace metternich {

class calendar final : public data_entry, public data_type<calendar>
{
	Q_OBJECT

	Q_PROPERTY(metternich::calendar* base_calendar MEMBER base_calendar WRITE set_base_calendar)
	Q_PROPERTY(int year_offset MEMBER year_offset READ get_year_offset)

public:
	static constexpr const char *class_identifier = "calendar";
	static constexpr const char *database_folder = "calendars";

	calendar(const std::string &identifier) : data_entry(identifier) {}

	virtual void initialize() override
	{
		if (this->base_calendar != nullptr) {
			if (!this->base_calendar->is_initialized()) {
				this->base_calendar->initialize();
			}

			//calculate the year offset to the Gregorian calendar based on the offset to the base calendar
			this->year_offset += this->base_calendar->get_year_offset();
			this->base_calendar = nullptr;
		}

		data_entry_base::initialize();
	}

	void set_base_calendar(calendar *base_calendar)
	{
		if (base_calendar == this->base_calendar) {
			return;
		}

		if (base_calendar == this) {
			throw std::runtime_error("Tried to set the base calendar of \"" + this->get_identifier() + "\" to itself.");
		}

		if (base_calendar->is_any_base_calendar(this)) {
			throw std::runtime_error("Tried to set the base calendar of \"" + this->get_identifier() + "\" to \"" + base_calendar->get_identifier() + "\", but that would create a circular dependency.");
		}

		this->base_calendar = base_calendar;
	}

	bool is_any_base_calendar(calendar *calendar) const
	{
		if (this->base_calendar == nullptr) {
			return false;
		}

		if (this->base_calendar == calendar) {
			return true;
		}

		return this->base_calendar->is_any_base_calendar(calendar);
	}

	int get_year_offset() const
	{
		return this->year_offset;
	}

private:
	calendar *base_calendar = nullptr; //the base calendar, used to ultimately calculate the year offset to the Gregorian calendar from
	int year_offset = 0; //the offset from the Gregorian calendar, in years
};

}
