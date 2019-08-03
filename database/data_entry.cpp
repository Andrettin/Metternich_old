#include "database/data_entry.h"

#include "character.h"
#include "culture.h"
#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "dynasty.h"
#include "game.h"
#include "history/history.h"
#include "landed_title.h"
#include "religion.h"
#include "trait.h"
#include "translator.h"
#include "util.h"

#include <QMetaProperty>

namespace Metternich {

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void DataEntryBase::ProcessGSMLProperty(const GSMLProperty &property)
{
	const QMetaObject *meta_object = this->metaObject();
	const int property_count = meta_object->propertyCount();
	for (int i = 0; i < property_count; ++i) {
		QMetaProperty meta_property = meta_object->property(i);
		const char *property_name = meta_property.name();

		if (property_name != property.GetKey()) {
			continue;
		}

		QVariant::Type property_type = meta_property.type();

		QVariant new_property_value;
		if (property_type == QVariant::Bool) {
			if (property.GetOperator() != GSMLOperator::Assignment) {
				throw std::runtime_error("Only the assignment operator is available for boolean properties.");
			}

			new_property_value = StringToBool(property.GetValue());
		} else if (property_type == QVariant::Int) {
			int value = std::stoi(property.GetValue());

			if (property.GetOperator() == GSMLOperator::Addition) {
				value = this->property(property_name).toInt() + value;
			} else if (property.GetOperator() == GSMLOperator::Subtraction) {
				value = this->property(property_name).toInt() - value;
			}

			new_property_value = value;
		} else if (property_type == QVariant::String) {
			if (property.GetOperator() != GSMLOperator::Assignment) {
				throw std::runtime_error("Only the assignment operator is available for string properties.");
			}

			new_property_value = QString::fromStdString(property.GetValue());
		} else if (property_type == QVariant::Type::UserType) {
			if (property.GetOperator() != GSMLOperator::Assignment) {
				throw std::runtime_error("Only the assignment operator is available for object reference properties.");
			}

			if (property.GetKey() == "landed_title" || property.GetKey() == "barony" || property.GetKey() == "county" || property.GetKey() == "duchy" || property.GetKey() == "kingdom" || property.GetKey() == "empire") {
				new_property_value = QVariant::fromValue(LandedTitle::Get(property.GetValue()));
			} else if (property.GetKey() == "culture") {
				new_property_value = QVariant::fromValue(Culture::Get(property.GetValue()));
			} else if (property.GetKey() == "religion") {
				new_property_value = QVariant::fromValue(Religion::Get(property.GetValue()));
			} else if (property.GetKey() == "dynasty") {
				new_property_value = QVariant::fromValue(Dynasty::Get(property.GetValue()));
			} else if (property.GetKey() == "character" || property.GetKey() == "holder" || property.GetKey() == "father" || property.GetKey() == "mother" || property.GetKey() == "spouse" || property.GetKey() == "liege" || property.GetKey() == "employer") {
				new_property_value = QVariant::fromValue(Character::Get(std::stoi(property.GetValue())));
			} else {
				throw std::runtime_error("Unknown type for object reference property \"" + std::string(property_name) + "\": \"" + property.GetKey() + "\".");
			}
		} else if (property_type == QVariant::Type::List) {
			if (property.GetOperator() == GSMLOperator::Assignment) {
				throw std::runtime_error("The assignment operator is not available for list properties.");
			}

			std::string method_name;
			if (property.GetOperator() == GSMLOperator::Addition) {
				method_name = "Add";
			} else if (property.GetOperator() == GSMLOperator::Subtraction) {
				method_name = "Remove";
			}

			method_name += GetSingularForm(SnakeCaseToPascalCase(property.GetKey()));

			bool success = false;
			if (property.GetKey() == "traits") {
				Trait *trait = Trait::Get(property.GetValue());
				success = QMetaObject::invokeMethod(this, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(Trait *, trait));
			} else {
				throw std::runtime_error("Unknown type for list property \"" + std::string(property_name) + "\": \"" + property.GetKey() + "\".");
			}

			if (!success) {
				throw std::runtime_error("Failed to add or remove value for list property \"" + std::string(property_name) + "\".");
			}

			return;
		} else {
			throw std::runtime_error("Invalid type for property \"" + std::string(property_name) + "\": \"" + std::string(meta_property.typeName()) + "\".");
		}

		bool success = this->setProperty(property_name, new_property_value);
		if (!success) {
			throw std::runtime_error("Failed to set value for property \"" + std::string(property_name) + "\".");
		}

		return;
	}

	throw std::runtime_error("Invalid " + std::string(meta_object->className()) + " property: \"" + property.GetKey() + "\".");
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
				this->ProcessGSMLHistoryProperty(property, date);
			}
		}
	}
}

std::string DataEntry::GetName() const
{
	return Translator::GetInstance()->Translate(this->Identifier);
}

}
