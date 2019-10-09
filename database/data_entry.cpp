#include "database/data_entry.h"

#include "database/database.h"
#include "game/game.h"
#include "history/history.h"
#include "translator.h"
#include "util.h"

namespace metternich {

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void data_entry_base::process_gsml_property(const gsml_property &property)
{
	database::process_gsml_property_for_object(this, property);
}

void data_entry_base::process_gsml_scope(const gsml_data &scope)
{
	const QMetaObject *meta_object = this->metaObject();
	throw std::runtime_error("Invalid \"" + util::pascal_case_to_snake_case(meta_object->className()) + "\" field: \"" + scope.get_tag() + "\".");
}

/**
**	@brief	Load history for the data entry
*/
void data_entry_base::load_history(gsml_data &data)
{
	for (const gsml_property &property : data.get_properties()) {
		this->process_gsml_property(property); //properties outside of a date scope, to be applied regardless of start date
	}

	data.sort_children(); //sort by date, so that they are applied chronologically

	for (const gsml_data &history_entry : data.get_children()) {
		QDateTime date = History::StringToDate(history_entry.get_tag());

		if (date <= Game::get()->GetCurrentDate()) {
			for (const gsml_property &property : history_entry.get_properties()) {
				this->process_gsml_dated_property(property, date);
			}

			for (const gsml_data &scope : history_entry.get_children()) {
				this->process_gsml_dated_scope(scope, date);
			}
		}
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
	return Translator::get()->Translate(this->identifier);
}

}
