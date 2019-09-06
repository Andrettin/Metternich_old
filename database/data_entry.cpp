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
void DataEntryBase::ProcessGSMLProperty(const GSMLProperty &property)
{
	Database::ProcessGSMLPropertyForObject(this, property);
}

void DataEntryBase::ProcessGSMLScope(const gsml_data &scope)
{
	const QMetaObject *meta_object = this->metaObject();
	throw std::runtime_error("Invalid \"" + PascalCaseToSnakeCase(meta_object->className()) + "\" field: \"" + scope.get_tag() + "\".");
}

/**
**	@brief	Load history for the data entry
*/
void DataEntryBase::LoadHistory(gsml_data &data)
{
	for (const GSMLProperty &property : data.get_properties()) {
		this->ProcessGSMLProperty(property); //properties outside of a date scope, to be applied regardless of start date
	}

	data.sort_children(); //sort by date, so that they are applied chronologically

	for (const gsml_data &history_entry : data.get_children()) {
		QDateTime date = History::StringToDate(history_entry.get_tag());

		if (date <= Game::Get()->GetCurrentDate()) {
			for (const GSMLProperty &property : history_entry.get_properties()) {
				this->ProcessGSMLDatedProperty(property, date);
			}

			for (const gsml_data &scope : history_entry.get_children()) {
				this->ProcessGSMLDatedScope(scope, date);
			}
		}
	}
}

std::string DataEntry::GetName() const
{
	return Translator::Get()->Translate(this->Identifier);
}

}
