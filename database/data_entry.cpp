#include "database/data_entry.h"

#include "database/database.h"
#include "game/game.h"
#include "history/calendar.h"
#include "history/history.h"
#include "history/timeline.h"
#include "translator.h"
#include "util/string_util.h"

namespace metternich {

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void data_entry_base::process_gsml_property(const gsml_property &property)
{
	if (property.get_key() == "aliases") {
		return; //alias addition is already handled in the data type class
	}

	database::process_gsml_property_for_object(this, property);
}

void data_entry_base::process_gsml_scope(const gsml_data &scope)
{
	const QMetaObject *meta_object = this->metaObject();
	throw std::runtime_error("Invalid \"" + string::pascal_case_to_snake_case(meta_object->className()) + "\" field: \"" + scope.get_tag() + "\".");
}

/**
**	@brief	Process history for the data entry
*/
void data_entry_base::process_history(const gsml_data &data)
{
	for (const gsml_property &property : data.get_properties()) {
		this->process_gsml_property(property); //properties outside of a date scope, to be applied regardless of start date
	}

	for (const gsml_data &history_entry : data.get_children()) {
		const timeline *timeline = nullptr;
		const calendar *calendar = nullptr;

		if (!std::isdigit(history_entry.get_tag().front()) && history_entry.get_tag().front() != '-') {
			timeline = timeline::try_get(history_entry.get_tag());

			if (timeline == nullptr) {
				calendar = calendar::try_get(history_entry.get_tag());
			}
		}

		if (timeline != nullptr) {
			if (history::get()->get_timeline() == nullptr || (history::get()->get_timeline() != timeline && !history::get()->get_timeline()->derives_from_timeline(timeline))) {
				continue;
			}

			for (const gsml_data &timeline_entry : history_entry.get_children()) {
				QDateTime date = history::string_to_date(timeline_entry.get_tag());
				if (history::get()->contains_timeline_date(timeline, date)) {
					this->history_entries[date].push_back(&timeline_entry);
				}
			}
		} else if (calendar != nullptr) {
			for (const gsml_data &calendar_entry : history_entry.get_children()) {
				QDateTime date = history::string_to_date(calendar_entry.get_tag());
				date = date.addYears(calendar->get_year_offset());

				if (history::get()->contains_timeline_date(nullptr, date)) {
					this->history_entries[date].push_back(&calendar_entry);
				}
			}
		} else {
			QDateTime date = history::string_to_date(history_entry.get_tag());

			if (history::get()->contains_timeline_date(nullptr, date)) {
				this->history_entries[date].push_back(&history_entry);
			}
		}
	}
}

/**
**	@brief	Load history for the data entry
*/
void data_entry_base::load_history()
{
	for (const auto &kv_pair : this->history_entries) {
		const QDateTime &date = kv_pair.first;
		const std::vector<const gsml_data *> &date_history_entries = kv_pair.second;

		for (const gsml_data *history_entry : date_history_entries) {
			this->load_date_scope(*history_entry, date);
		}
	}

	this->history_entries.clear();
}

/**
**	@brief	Load date scope for the data entry
*/
void data_entry_base::load_date_scope(const gsml_data &date_scope, const QDateTime &date)
{
	for (const gsml_property &property : date_scope.get_properties()) {
		this->process_gsml_dated_property(property, date);
	}

	for (const gsml_data &scope : date_scope.get_children()) {
		this->process_gsml_dated_scope(scope, date);
	}
}

/**
**	@brief	Get cache data for the data entry
*/
gsml_data identifiable_data_entry_base::get_cache_data() const
{
	throw std::runtime_error("The data entry's type does not support caching data.");
}

std::string data_entry::get_name() const
{
	return translator::get()->translate(this->get_identifier_with_aliases());
}

}
