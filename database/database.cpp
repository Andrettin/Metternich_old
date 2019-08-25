#include "database/database.h"

#include "character/character.h"
#include "character/dynasty.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "defines.h"
#include "economy/commodity.h"
#include "game/game.h"
#include "history/history.h"
#include "holding/holding.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "map/region.h"
#include "map/terrain.h"
#include "population/population_type.h"
#include "religion.h"
#include "translator.h"
#include "util.h"

#include <QMetaProperty>
#include <QObject>

namespace Metternich {

/**
**	@brief	Process a GSML property for an instance of a QObject-derived class
**
**	@param	object		The object
**	@param	property	The property
*/
void Database::ProcessGSMLPropertyForObject(QObject *object, const GSMLProperty &property)
{
	const QMetaObject *meta_object = object->metaObject();
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
			int value = 0;

			if (property.GetKey() == "efficiency" || property.GetKey() == "output_value" || property.GetKey() == "output_modifier" || property.GetKey() == "workforce_proportion" || property.GetKey() == "proportion_to_workforce" || property.GetKey() == "income_share") {
				value = CentesimalNumberStringToInt(property.GetValue());
			} else if (property.GetKey() == "base_population_growth") {
				value = FractionalNumberStringToInt<4>(property.GetValue());
			} else {
				value = std::stoi(property.GetValue());
			}

			if (property.GetOperator() == GSMLOperator::Addition) {
				value = object->property(property_name).toInt() + value;
			} else if (property.GetOperator() == GSMLOperator::Subtraction) {
				value = object->property(property_name).toInt() - value;
			}

			new_property_value = value;
		} else if (property_type == QVariant::String) {
			if (property.GetOperator() != GSMLOperator::Assignment) {
				throw std::runtime_error("Only the assignment operator is available for string properties.");
			}

			new_property_value = QString::fromStdString(property.GetValue());
		} else if (property_type == QVariant::DateTime) {
			if (property.GetOperator() != GSMLOperator::Assignment) {
				throw std::runtime_error("Only the assignment operator is available for date-time properties.");
			}

			new_property_value = History::StringToDate(property.GetValue());
		} else if (property_type == QVariant::Type::UserType) {
			if (property.GetOperator() != GSMLOperator::Assignment) {
				throw std::runtime_error("Only the assignment operator is available for object reference properties.");
			}

			if (property.GetKey() == "landed_title" || property.GetKey() == "barony" || property.GetKey() == "county" || property.GetKey() == "duchy" || property.GetKey() == "kingdom" || property.GetKey() == "empire" || property.GetKey() == "holder_title" || property.GetKey() == "liege_title" || property.GetKey() == "de_jure_liege_title") {
				new_property_value = QVariant::fromValue(LandedTitle::Get(property.GetValue()));
			} else if (property.GetKey() == "province" || property.GetKey() == "capital_province") {
				Province *province = Province::Get(property.GetValue());
				new_property_value = QVariant::fromValue(province);
			} else if (property.GetKey() == "holding" || property.GetKey() == "capital_holding") {
				const LandedTitle *barony = LandedTitle::Get(property.GetValue());
				Holding *holding = barony->GetHolding();
				if (holding == nullptr) {
					throw std::runtime_error("Barony \"" + property.GetValue() + "\" has no holding, but a holding property is being set using the barony as the holding's identifier.");
				}
				new_property_value = QVariant::fromValue(holding);
			} else if (property.GetKey() == "region") {
				new_property_value = QVariant::fromValue(Region::Get(property.GetValue()));
			} else if (property.GetKey() == "terrain") {
				new_property_value = QVariant::fromValue(Terrain::Get(property.GetValue()));
			} else if (property.GetKey() == "culture") {
				new_property_value = QVariant::fromValue(Culture::Get(property.GetValue()));
			} else if (property.GetKey() == "culture_group") {
				new_property_value = QVariant::fromValue(CultureGroup::Get(property.GetValue()));
			} else if (property.GetKey() == "religion") {
				new_property_value = QVariant::fromValue(Religion::Get(property.GetValue()));
			} else if (property.GetKey() == "dynasty") {
				new_property_value = QVariant::fromValue(Dynasty::Get(property.GetValue()));
			} else if (property.GetKey() == "character" || property.GetKey() == "holder" || property.GetKey() == "father" || property.GetKey() == "mother" || property.GetKey() == "spouse" || property.GetKey() == "liege" || property.GetKey() == "employer") {
				new_property_value = QVariant::fromValue(Character::Get(std::stoi(property.GetValue())));
			} else if (property.GetKey() == "commodity" || property.GetKey() == "output_commodity") {
				new_property_value = QVariant::fromValue(Commodity::Get(property.GetValue()));
			} else {
				throw std::runtime_error("Unknown type for object reference property \"" + std::string(property_name) + "\".");
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
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(Trait *, trait));
			} else if (property.GetKey() == "holding_types") {
				HoldingType *holding_type = HoldingType::Get(property.GetValue());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(HoldingType *, holding_type));
			} else if (property.GetKey() == "provinces") {
				Province *province = Province::Get(property.GetValue());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(Province *, province));
			} else if (property.GetKey() == "subregions") {
				Region *region = Region::Get(property.GetValue());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(Region *, region));
			} else if (property.GetKey() == "discount_types") {
				PopulationType *type = PopulationType::Get(property.GetValue());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(PopulationType *, type));
			} else {
				throw std::runtime_error("Unknown type for list property \"" + std::string(property_name) + "\".");
			}

			if (!success) {
				throw std::runtime_error("Failed to add or remove value for list property \"" + std::string(property_name) + "\".");
			}

			return;
		} else {
			throw std::runtime_error("Invalid type for property \"" + std::string(property_name) + "\": \"" + std::string(meta_property.typeName()) + "\".");
		}

		bool success = object->setProperty(property_name, new_property_value);
		if (!success) {
			throw std::runtime_error("Failed to set value for property \"" + std::string(property_name) + "\".");
		}

		return;
	}

	throw std::runtime_error("Invalid " + std::string(meta_object->className()) + " property: \"" + property.GetKey() + "\".");
}

/**
**	@brief	Load the database
*/
void Database::Load()
{
	Defines::Get()->Load();

	//parse the files for in each data type's folder
	for (const std::function<void()> &function : this->ParsingFunctions) {
		function();
	}

	//create data entries for each data type
	for (const std::function<void(bool)> &function : this->ProcessingFunctions) {
		function(true);
	}

	//actually define the data entries for each data type
	for (const std::function<void(bool)> &function : this->ProcessingFunctions) {
		function(false);
	}

	//initialize data entries for each data type
	for (const std::function<void()> &function : this->InitializationFunctions) {
		function();
	}

	//check if data entries are valid for each data type
	for (const std::function<void()> &function : this->CheckingFunctions) {
		function();
	}
}

}
