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
#include "economy/employment_type.h"
#include "engine_interface.h"
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

namespace metternich {

/**
**	@brief	Process a GSML property for an instance of a QObject-derived class
**
**	@param	object		The object
**	@param	property	The property
*/
void Database::ProcessGSMLPropertyForObject(QObject *object, const gsml_property &property)
{
	const QMetaObject *meta_object = object->metaObject();
	const int property_count = meta_object->propertyCount();
	for (int i = 0; i < property_count; ++i) {
		QMetaProperty meta_property = meta_object->property(i);
		const char *property_name = meta_property.name();

		if (property_name != property.get_key()) {
			continue;
		}

		QVariant::Type property_type = meta_property.type();

		QVariant new_property_value;
		if (property_type == QVariant::Bool) {
			if (property.get_operator() != gsml_operator::assignment) {
				throw std::runtime_error("Only the assignment operator is available for boolean properties.");
			}

			new_property_value = util::string_to_bool(property.get_value());
		} else if (property_type == QVariant::Int) {
			int value = 0;

			if (property.get_key() == "efficiency" || property.get_key() == "output_value" || property.get_key() == "output_modifier" || property.get_key() == "workforce_proportion" || property.get_key() == "proportion_to_workforce" || property.get_key() == "income_share") {
				value = util::centesimal_number_string_to_int(property.get_value());
			} else if (property.get_key() == "base_population_growth") {
				value = util::fractional_number_string_to_int<4>(property.get_value());
			} else {
				value = std::stoi(property.get_value());
			}

			if (property.get_operator() == gsml_operator::addition) {
				value = object->property(property_name).toInt() + value;
			} else if (property.get_operator() == gsml_operator::subtraction) {
				value = object->property(property_name).toInt() - value;
			}

			new_property_value = value;
		} else if (property_type == QVariant::String) {
			if (property.get_operator() != gsml_operator::assignment) {
				throw std::runtime_error("Only the assignment operator is available for string properties.");
			}

			new_property_value = QString::fromStdString(property.get_value());
		} else if (property_type == QVariant::DateTime) {
			if (property.get_operator() != gsml_operator::assignment) {
				throw std::runtime_error("Only the assignment operator is available for date-time properties.");
			}

			new_property_value = History::StringToDate(property.get_value());
		} else if (property_type == QVariant::Type::UserType) {
			if (property.get_operator() != gsml_operator::assignment) {
				throw std::runtime_error("Only the assignment operator is available for object reference properties.");
			}

			if (property.get_key() == "landed_title" || property.get_key() == "barony" || property.get_key() == "county" || property.get_key() == "duchy" || property.get_key() == "kingdom" || property.get_key() == "empire" || property.get_key() == "holder_title" || property.get_key() == "liege_title" || property.get_key() == "de_jure_liege_title") {
				new_property_value = QVariant::fromValue(LandedTitle::Get(property.get_value()));
			} else if (property.get_key() == "province" || property.get_key() == "capital_province") {
				Province *province = Province::Get(property.get_value());
				new_property_value = QVariant::fromValue(province);
			} else if (property.get_key() == "holding" || property.get_key() == "capital_holding") {
				const LandedTitle *barony = LandedTitle::Get(property.get_value());
				holding *holding = barony->get_holding();
				if (holding == nullptr) {
					throw std::runtime_error("Barony \"" + property.get_value() + "\" has no holding, but a holding property is being set using the barony as the holding's identifier.");
				}
				new_property_value = QVariant::fromValue(holding);
			} else if (property.get_key() == "region") {
				new_property_value = QVariant::fromValue(Region::Get(property.get_value()));
			} else if (property.get_key() == "terrain") {
				new_property_value = QVariant::fromValue(Terrain::Get(property.get_value()));
			} else if (property.get_key() == "culture") {
				new_property_value = QVariant::fromValue(Culture::Get(property.get_value()));
			} else if (property.get_key() == "culture_group") {
				new_property_value = QVariant::fromValue(CultureGroup::Get(property.get_value()));
			} else if (property.get_key() == "religion") {
				new_property_value = QVariant::fromValue(Religion::Get(property.get_value()));
			} else if (property.get_key() == "dynasty") {
				new_property_value = QVariant::fromValue(Dynasty::Get(property.get_value()));
			} else if (property.get_key() == "character" || property.get_key() == "holder" || property.get_key() == "father" || property.get_key() == "mother" || property.get_key() == "spouse" || property.get_key() == "liege" || property.get_key() == "employer") {
				new_property_value = QVariant::fromValue(Character::Get(std::stoi(property.get_value())));
			} else if (property.get_key() == "commodity" || property.get_key() == "output_commodity") {
				new_property_value = QVariant::fromValue(Commodity::Get(property.get_value()));
			} else if (property.get_key() == "employment_type") {
				new_property_value = QVariant::fromValue(EmploymentType::Get(property.get_value()));
			} else {
				throw std::runtime_error("Unknown type for object reference property \"" + std::string(property_name) + "\".");
			}
		} else if (property_type == QVariant::Type::List) {
			if (property.get_operator() == gsml_operator::assignment) {
				throw std::runtime_error("The assignment operator is not available for list properties.");
			}

			std::string method_name;
			if (property.get_operator() == gsml_operator::addition) {
				method_name = "Add";
			} else if (property.get_operator() == gsml_operator::subtraction) {
				method_name = "Remove";
			}

			method_name += util::get_singular_form(util::snake_case_to_pascal_case(property.get_key()));

			bool success = false;
			if (property.get_key() == "traits") {
				Trait *trait = Trait::Get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(Trait *, trait));
			} else if (property.get_key() == "holding_types") {
				holding_type *holding_type_value = holding_type::Get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(holding_type *, holding_type_value));
			} else if (property.get_key() == "provinces") {
				Province *province = Province::Get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(Province *, province));
			} else if (property.get_key() == "subregions") {
				Region *region = Region::Get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(Region *, region));
			} else if (property.get_key() == "discount_types") {
				PopulationType *type = PopulationType::Get(property.get_value());
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

	throw std::runtime_error("Invalid " + std::string(meta_object->className()) + " property: \"" + property.get_key() + "\".");
}

/**
**	@brief	Load the database
*/
void Database::Load()
{
	EngineInterface::Get()->SetLoadingMessage("Loading Database...");

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
