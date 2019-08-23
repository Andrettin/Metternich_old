#include "database/data_entry.h"

#include "database/database.h"
#include "game/game.h"
#include "history/history.h"
#include "translator.h"
#include "util.h"

namespace Metternich {

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void DataEntryBase::ProcessGSMLProperty(const GSMLProperty &property)
{
	Database::ProcessGSMLPropertyForObject(this, property);
}

void DataEntryBase::ProcessGSMLScope(const GSMLData &scope)
{
	const QMetaObject *meta_object = this->metaObject();
	throw std::runtime_error("Invalid \"" + PascalCaseToSnakeCase(meta_object->className()) + "\" field: \"" + scope.GetTag() + "\".");
}

/**
**	@brief	Load history for the data entry
*/
void DataEntryBase::LoadHistory(GSMLData &gsml_data)
{
	for (const GSMLProperty &property : gsml_data.GetProperties()) {
		this->ProcessGSMLProperty(property); //properties outside of a date scope, to be applied regardless of start date
	}

	gsml_data.SortChildren(); //sort by date, so that they are applied chronologically

	for (const GSMLData &history_entry : gsml_data.GetChildren()) {
		QDateTime date = History::StringToDate(history_entry.GetTag());

		if (date <= Game::GetInstance()->GetCurrentDate()) {
			for (const GSMLProperty &property : history_entry.GetProperties()) {
				this->ProcessGSMLDatedProperty(property, date);
			}

			for (const GSMLData &scope : history_entry.GetChildren()) {
				this->ProcessGSMLDatedScope(scope, date);
			}
		}
	}
}

std::string DataEntry::GetName() const
{
	return Translator::GetInstance()->Translate(this->Identifier);
}

}
