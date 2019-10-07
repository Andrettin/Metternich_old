#include "database/database.h"

#include "character/character.h"
#include "character/dynasty.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/data_type_metadata.h"
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
#include "map/terrain_type.h"
#include "phenotype.h"
#include "politics/law.h"
#include "politics/law_group.h"
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
void database::process_gsml_property_for_object(QObject *object, const gsml_property &property)
{
	const QMetaObject *meta_object = object->metaObject();
	const std::string class_name = meta_object->className();
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

			if (property.get_key() == "efficiency" || property.get_key() == "output_value" || property.get_key() == "output_modifier" || property.get_key() == "workforce_proportion" || property.get_key() == "proportion_to_workforce" || property.get_key() == "income_share" || property.get_key() == "base_price") {
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
				new_property_value = QVariant::fromValue(LandedTitle::get(property.get_value()));
			} else if (property.get_key() == "province" || property.get_key() == "capital_province") {
				province *province = province::get(property.get_value());
				new_property_value = QVariant::fromValue(province);
			} else if (property.get_key() == "holding" || property.get_key() == "capital_holding") {
				const LandedTitle *barony = LandedTitle::get(property.get_value());
				holding *holding = barony->get_holding();
				if (holding == nullptr) {
					throw std::runtime_error("Barony \"" + property.get_value() + "\" has no holding, but a holding property is being set using the barony as the holding's identifier.");
				}
				new_property_value = QVariant::fromValue(holding);
			} else if (property.get_key() == "region") {
				new_property_value = QVariant::fromValue(region::get(property.get_value()));
			} else if (property.get_key() == "terrain") {
				new_property_value = QVariant::fromValue(terrain_type::get(property.get_value()));
			} else if (property.get_key() == "culture") {
				new_property_value = QVariant::fromValue(culture::get(property.get_value()));
			} else if (property.get_key() == "culture_group") {
				new_property_value = QVariant::fromValue(culture_group::get(property.get_value()));
			} else if (property.get_key() == "religion") {
				new_property_value = QVariant::fromValue(religion::get(property.get_value()));
			} else if (property.get_key() == "phenotype" || property.get_key() == "default_phenotype") {
				new_property_value = QVariant::fromValue(phenotype::get(property.get_value()));
			} else if (property.get_key() == "dynasty") {
				new_property_value = QVariant::fromValue(Dynasty::get(property.get_value()));
			} else if (property.get_key() == "character" || property.get_key() == "holder" || property.get_key() == "father" || property.get_key() == "mother" || property.get_key() == "spouse" || property.get_key() == "liege" || property.get_key() == "employer") {
				new_property_value = QVariant::fromValue(Character::get(std::stoi(property.get_value())));
			} else if (property.get_key() == "commodity" || property.get_key() == "output_commodity") {
				new_property_value = QVariant::fromValue(commodity::get(property.get_value()));
			} else if (property.get_key() == "employment_type") {
				new_property_value = QVariant::fromValue(employment_type::get(property.get_value()));
			} else if (property.get_key() == "group") {
				if (class_name == "metternich::law") {
					new_property_value = QVariant::fromValue(law_group::get_or_add(property.get_value()));
				} else {
					throw std::runtime_error("Unknown type for object reference property \"" + std::string(property_name) + "\".");
				}
			} else {
				throw std::runtime_error("Unknown type for object reference property \"" + std::string(property_name) + "\".");
			}
		} else if (property_type == QVariant::Type::List) {
			if (property.get_operator() == gsml_operator::assignment) {
				throw std::runtime_error("The assignment operator is not available for list properties.");
			}

			std::string method_name;
			if (property.get_operator() == gsml_operator::addition) {
				method_name = "add_";
			} else if (property.get_operator() == gsml_operator::subtraction) {
				method_name = "remove_";
			}

			method_name += util::get_singular_form(property.get_key());

			bool success = false;
			if (property.get_key() == "traits") {
				Trait *trait = Trait::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(Trait *, trait));
			} else if (property.get_key() == "holdings") {
				LandedTitle *barony = LandedTitle::get(property.get_value());
				if (class_name == "metternich::region") {
					success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(LandedTitle *, barony));
				} else {
					holding *holding = barony->get_holding();
					if (holding == nullptr) {
						throw std::runtime_error("Barony \"" + property.get_value() + "\" has no holding, but a holding list property is being modified using the barony as a holding's identifier.");
					}
					success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(metternich::holding *, holding));
				}
			} else if (property.get_key() == "holding_types") {
				holding_type *holding_type_value = holding_type::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(holding_type *, holding_type_value));
			} else if (property.get_key() == "provinces") {
				province *province_value = province::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(province *, province_value));
			} else if (property.get_key() == "subregions") {
				region *region = region::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(metternich::region *, region));
			} else if (property.get_key() == "discount_types") {
				PopulationType *type = PopulationType::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(PopulationType *, type));
			} else if (property.get_key() == "laws") {
				law *law = law::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(metternich::law *, law));
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
**	@brief	Constructor
*/
database::database()
{
}

/**
**	@brief	Destructor
*/
database::~database()
{
}

/**
**	@brief	Load the database
*/
void database::load()
{
	EngineInterface::get()->set_loading_message("Loading Database...");

	Defines::get()->load();

	//sort the metadata instances so they are placed after their class' dependencies' metadata
	std::sort(this->metadata.begin(), this->metadata.end(), [](const std::unique_ptr<data_type_metadata> &a, const std::unique_ptr<data_type_metadata> &b) {
		if (a->has_database_dependency_on(b)) {
			return false;
		} else if (b->has_database_dependency_on(a)) {
			return true;
		}

		return a.get() < b.get();
	});

	//parse the files for in each data type's folder
	for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
		metadata->get_parsing_function()();
	}

	//create data entries for each data type
	for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
		metadata->get_processing_function()(true);
	}

	//actually define the data entries for each data type
	for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
		metadata->get_processing_function()(false);
	}

	//initialize data entries for each data type
	for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
		metadata->get_initialization_function()();
	}

	//check if data entries are valid for each data type
	for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
		metadata->get_checking_function()();
	}
}

void database::initialize_history()
{
	//initialize data entries are valid for each data type
	for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
		metadata->get_history_initialization_function()();
	}

	//check if data entries are valid for each data type
	for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
		metadata->get_checking_function()();
	}
}

void database::register_metadata(std::unique_ptr<data_type_metadata> &&metadata)
{
	this->metadata.push_back(std::move(metadata));
}

}
