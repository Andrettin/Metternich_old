#include "database/database.h"

#include "character/character.h"
#include "character/dynasty.h"
#include "character/item.h"
#include "character/trait.h"
#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/data_type_metadata.h"
#include "database/gsml_data.h"
#include "database/gsml_operator.h"
#include "database/gsml_property.h"
#include "database/module.h"
#include "defines.h"
#include "economy/commodity.h"
#include "economy/employment_type.h"
#include "economy/trade_node.h"
#include "engine_interface.h"
#include "game/game.h"
#include "history/calendar.h"
#include "history/history.h"
#include "history/timeline.h"
#include "holding/holding.h"
#include "holding/holding_slot.h"
#include "holding/holding_slot_type.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/province.h"
#include "map/province_profile.h"
#include "map/region.h"
#include "map/terrain_type.h"
#include "map/world.h"
#include "phenotype.h"
#include "politics/government_type.h"
#include "politics/government_type_group.h"
#include "politics/law.h"
#include "politics/law_group.h"
#include "population/population_type.h"
#include "religion/religion.h"
#include "religion/religion_group.h"
#include "species/species.h"
#include "technology/technology.h"
#include "technology/technology_category.h"
#include "translator.h"
#include "util/parse_util.h"
#include "util/string_util.h"

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

		if (property_type == QVariant::Type::List) {
			if (property.get_operator() == gsml_operator::assignment) {
				throw std::runtime_error("The assignment operator is not available for list properties.");
			}

			std::string method_name;
			if (property.get_operator() == gsml_operator::addition) {
				method_name = "add_";
			} else if (property.get_operator() == gsml_operator::subtraction) {
				method_name = "remove_";
			}

			method_name += string::get_singular_form(property.get_key());

			bool success = false;

			if (property.get_key() == "dependencies") {
				module *module_value = database::get()->get_module(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(module *, module_value));
			} else if (property.get_key() == "derived_cultures") {
				culture *culture_value = culture::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(culture *, culture_value));
			} else if (property.get_key() == "holdings") {
				holding_slot *slot = holding_slot::get(property.get_value());
				if (class_name == "metternich::region") {
					success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(holding_slot *, slot));
				} else {
					holding *holding_value = slot->get_holding();
					if (holding_value == nullptr) {
						throw std::runtime_error("Holding slot \"" + property.get_value() + "\" has no constructed holding, but a holding property is being set using it as the holding's identifier.");
					}
					success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(holding *, holding_value));
				}
			} else if (property.get_key() == "holding_types" || property.get_key() == "allowed_holding_types") {
				holding_type *holding_type_value = holding_type::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(holding_type *, holding_type_value));
			} else if (property.get_key() == "laws" || property.get_key() == "default_laws") {
				law *law_value = law::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(law *, law_value));
			} else if (property.get_key() == "discount_types") {
				population_type *population_type_value = population_type::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(population_type *, population_type_value));
			} else if (property.get_key() == "items") {
				item *item_value = item::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(item *, item_value));
			} else if (property.get_key() == "provinces") {
				province *province_value = province::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(province *, province_value));
			} else if (property.get_key() == "species" || property.get_key() == "evolutions") {
				species *species_value = species::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(metternich::species *, species_value));
			} else if (property.get_key() == "subregions") {
				region *region_value = region::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(metternich::region *, region_value));
			} else if (property.get_key() == "technologies" || property.get_key() == "required_technologies") {
				technology *technology_value = technology::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(technology *, technology_value));
			} else if (property.get_key() == "traits") {
				trait *trait_value = trait::get(property.get_value());
				success = QMetaObject::invokeMethod(object, method_name.c_str(), Qt::ConnectionType::DirectConnection, Q_ARG(trait *, trait_value));
			} else {
				throw std::runtime_error("Unknown type for list property \"" + std::string(property_name) + "\" (in class \"" + class_name + "\").");
			}

			if (!success) {
				throw std::runtime_error("Failed to add or remove value for list property \"" + std::string(property_name) + "\".");
			}

			return;
		} else {
			QVariant new_property_value = database::process_gsml_property_value(property, meta_property, object);
			bool success = object->setProperty(property_name, new_property_value);
			if (!success) {
				throw std::runtime_error("Failed to set value for property \"" + std::string(property_name) + "\".");
			}
			return;
		}
	}

	throw std::runtime_error("Invalid " + std::string(meta_object->className()) + " property: \"" + property.get_key() + "\".");
}

QVariant database::process_gsml_property_value(const gsml_property &property, const QMetaProperty &meta_property, const QObject *object)
{
	const std::string class_name = meta_property.enclosingMetaObject()->className();
	const char *property_name = meta_property.name();
	const std::string property_class_name = meta_property.typeName();
	const QVariant::Type property_type = meta_property.type();

	QVariant new_property_value;
	if (property_type == QVariant::Bool) {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for boolean properties.");
		}

		new_property_value = string::to_bool(property.get_value());
	} else if (property_type == QVariant::Int) {
		int value = 0;

		if (property.get_key() == "efficiency" || property.get_key() == "output_value" || property.get_key() == "output_modifier" || property.get_key() == "workforce_proportion" || property.get_key() == "proportion_to_workforce" || property.get_key() == "income_share" || property.get_key() == "base_price" || property.get_key() == "trade_node_score_realm_modifier" || property.get_key() == "trade_node_score_culture_modifier" || property.get_key() == "trade_node_score_culture_group_modifier" || property.get_key() == "trade_node_score_religion_modifier" || property.get_key() == "trade_node_score_religion_group_modifier" || property.get_key() == "holding_size") {
			value = parse::centesimal_number_string_to_int(property.get_value());
		} else if (property.get_key() == "base_population_growth" || property.get_key() == "cultural_derivation_factor" || property.get_key() == "trade_cost_modifier_per_distance" || property.get_key() == "base_port_trade_cost_modifier") {
			value = parse::fractional_number_string_to_int<4>(property.get_value());
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

		new_property_value = history::string_to_date(property.get_value());
	} else if (property_type == QVariant::Type::UserType) {
		if (property.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for object reference properties.");
		}

		if (property.get_key() == "category") {
			if (class_name == "metternich::technology") {
				new_property_value = QVariant::fromValue(technology_category::get_or_add(property.get_value()));
			} else {
				throw std::runtime_error("Unknown type for object reference property \"" + std::string(property_name) + "\".");
			}
		} else if ((property.get_key() == "group" && class_name == "metternich::law") || property.get_key() == "succession_law_group") {
			new_property_value = QVariant::fromValue(law_group::get_or_add(property.get_value()));
		} else if (property_class_name == "metternich::calendar*") {
			new_property_value = QVariant::fromValue(calendar::get(property.get_value()));
		} else if (property_class_name == "metternich::character*") {
			new_property_value = QVariant::fromValue(character::get(property.get_value()));
		} else if (property_class_name == "metternich::commodity*") {
			new_property_value = QVariant::fromValue(commodity::get(property.get_value()));
		} else if (property_class_name == "metternich::culture*") {
			new_property_value = QVariant::fromValue(culture::get(property.get_value()));
		} else if (property_class_name == "metternich::culture_group*") {
			new_property_value = QVariant::fromValue(culture_group::get(property.get_value()));
		} else if (property_class_name == "metternich::dynasty*") {
			new_property_value = QVariant::fromValue(dynasty::get(property.get_value()));
		} else if (property_class_name == "metternich::employment_type*") {
			new_property_value = QVariant::fromValue(employment_type::get(property.get_value()));
		} else if (property_class_name == "metternich::government_type*") {
			new_property_value = QVariant::fromValue(government_type::get(property.get_value()));
		} else if (property_class_name == "metternich::government_type_group") {
			new_property_value = QVariant::fromValue(string_to_government_type_group(property.get_value()));
		} else if (property_class_name == "metternich::holding*") {
			const holding_slot *holding_slot = holding_slot::get(property.get_value());
			holding *holding = holding_slot->get_holding();
			if (holding == nullptr && class_name != "metternich::population_unit") {
				throw std::runtime_error("Holding slot \"" + property.get_value() + "\" has no constructed holding, but a holding property is being set using it as the holding's identifier.");
			}
			new_property_value = QVariant::fromValue(holding);
		} else if (property_class_name == "metternich::holding_slot*") {
			new_property_value = QVariant::fromValue(holding_slot::get(property.get_value()));
		} else if (property_class_name == "metternich::holding_slot_type") {
			new_property_value = QVariant::fromValue(string_to_holding_slot_type(property.get_value()));
		} else if (property_class_name == "metternich::holding_type*") {
			new_property_value = QVariant::fromValue(holding_type::get(property.get_value()));
		} else if (property_class_name == "metternich::item*") {
			new_property_value = QVariant::fromValue(item::get(property.get_value()));
		} else if (property_class_name == "metternich::landed_title*") {
			new_property_value = QVariant::fromValue(landed_title::get(property.get_value()));
		} else if (property_class_name == "metternich::law*") {
			new_property_value = QVariant::fromValue(law::get(property.get_value()));
		} else if (property_class_name == "metternich::module*") {
			new_property_value = QVariant::fromValue(database::get()->get_module(property.get_value()));
		} else if (property_class_name == "metternich::phenotype*") {
			new_property_value = QVariant::fromValue(phenotype::get(property.get_value()));
		} else if (property_class_name == "metternich::population_type*") {
			new_property_value = QVariant::fromValue(population_type::get(property.get_value()));
		} else if (property_class_name == "metternich::province*") {
			new_property_value = QVariant::fromValue(province::get(property.get_value()));
		} else if (property_class_name == "metternich::province_profile*") {
			new_property_value = QVariant::fromValue(province_profile::get(property.get_value()));
		} else if (property_class_name == "metternich::region*") {
			new_property_value = QVariant::fromValue(region::get(property.get_value()));
		} else if (property_class_name == "metternich::religion*") {
			new_property_value = QVariant::fromValue(religion::get(property.get_value()));
		} else if (property_class_name == "metternich::religion_group*") {
			new_property_value = QVariant::fromValue(religion_group::get(property.get_value()));
		} else if (property_class_name == "metternich::species*") {
			new_property_value = QVariant::fromValue(species::get(property.get_value()));
		} else if (property_class_name == "metternich::technology*") {
			new_property_value = QVariant::fromValue(technology::get(property.get_value()));
		} else if (property_class_name == "metternich::terrain_type*") {
			new_property_value = QVariant::fromValue(terrain_type::get(property.get_value()));
		} else if (property_class_name == "metternich::timeline*") {
			new_property_value = QVariant::fromValue(timeline::get(property.get_value()));
		} else if (property_class_name == "metternich::trade_node*") {
			new_property_value = QVariant::fromValue(trade_node::get(property.get_value()));
		} else if (property_class_name == "metternich::trait*") {
			new_property_value = QVariant::fromValue(trait::get(property.get_value()));
		} else if (property_class_name == "metternich::world*") {
			new_property_value = QVariant::fromValue(world::get(property.get_value()));
		} else {
			throw std::runtime_error("Unknown type for object reference property \"" + std::string(property_name) + "\" (\"" + property_class_name + "\").");
		}
	} else {
		throw std::runtime_error("Invalid type for property \"" + std::string(property_name) + "\": \"" + std::string(meta_property.typeName()) + "\".");
	}

	return new_property_value;
}

void database::process_gsml_scope_for_object(QObject *object, const gsml_data &scope)
{
	const QMetaObject *meta_object = object->metaObject();
	const std::string class_name = meta_object->className();
	const int property_count = meta_object->propertyCount();
	for (int i = 0; i < property_count; ++i) {
		QMetaProperty meta_property = meta_object->property(i);
		const char *property_name = meta_property.name();

		if (property_name != scope.get_tag()) {
			continue;
		}

		QVariant new_property_value = database::process_gsml_scope_value(scope, meta_property);
		const bool success = object->setProperty(property_name, new_property_value);
		if (!success) {
			throw std::runtime_error("Failed to set value for scope property \"" + std::string(property_name) + "\".");
		}

		return;
	}

	throw std::runtime_error("Invalid " + std::string(meta_object->className()) + " scope property: \"" + scope.get_tag() + "\".");
}

QVariant database::process_gsml_scope_value(const gsml_data &scope, const QMetaProperty &meta_property)
{
	const std::string class_name = meta_property.enclosingMetaObject()->className();
	const char *property_name = meta_property.name();
	const std::string property_class_name = meta_property.typeName();
	const QVariant::Type property_type = meta_property.type();

	QVariant new_property_value;
	if (property_type == QVariant::Color) {
		if (scope.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for color properties.");
		}

		new_property_value = scope.to_color();
	} else if (property_type == QVariant::Point) {
		if (scope.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for point properties.");
		}

		new_property_value = scope.to_point();
	} else if (property_class_name == "QGeoCoordinate") {
		if (scope.get_operator() != gsml_operator::assignment) {
			throw std::runtime_error("Only the assignment operator is available for geocoordinate properties.");
		}

		new_property_value = QVariant::fromValue(scope.to_geocoordinate());
	} else {
		throw std::runtime_error("Invalid type for scope property \"" + std::string(property_name) + "\": \"" + std::string(meta_property.typeName()) + "\".");
	}

	return new_property_value;
}

database::database()
{
}

database::~database()
{
}

void database::load()
{
	engine_interface::get()->set_loading_message("Loading Database...");

	this->process_icon_paths();
	this->process_holding_portrait_paths();
	this->process_flag_paths();

	//sort the metadata instances so they are placed after their class' dependencies' metadata
	std::sort(this->metadata.begin(), this->metadata.end(), [](const std::unique_ptr<data_type_metadata> &a, const std::unique_ptr<data_type_metadata> &b) {
		if (a->has_database_dependency_on(b)) {
			return false;
		} else if (b->has_database_dependency_on(a)) {
			return true;
		}

		return a->get_database_dependency_count() < b->get_database_dependency_count();
	});

	for (const std::filesystem::path &path : database::get()->get_data_paths()) {
		//parse the files in each data type's folder
		for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
			metadata->get_parsing_function()(path);
		}

		//create data entries for each data type
		for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
			metadata->get_processing_function()(true);
		}

		defines::get()->load(path); //load the defines here so that they can refer to data entries

		//actually define the data entries for each data type
		for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
			metadata->get_processing_function()(false);
		}
	}
}

void database::initialize()
{
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
	//initialize data entries' history for each data type
	for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
		metadata->get_history_initialization_function()();
	}

	//check if data entries are valid for each data type
	for (const std::unique_ptr<data_type_metadata> &metadata : this->metadata) {
		metadata->get_history_checking_function()();
	}
}

void database::register_metadata(std::unique_ptr<data_type_metadata> &&metadata)
{
	this->metadata.push_back(std::move(metadata));
}

void database::process_modules()
{
	this->process_modules_at_dir(database::get_modules_path());

	if (std::filesystem::exists(database::get_documents_modules_path())) {
		this->process_modules_at_dir(database::get_documents_modules_path());
	}

	for (const std::unique_ptr<module> &module : this->modules) {
		std::filesystem::path module_file = module->get_path() / "module.txt";

		if (std::filesystem::exists(module_file)) {
			gsml_parser parser(module_file);
			database::process_gsml_data(module, parser.parse());
		}
	}

	std::sort(this->modules.begin(), this->modules.end(), [](const std::unique_ptr<module> &a, const std::unique_ptr<module> &b) {
		if (a->depends_on(b.get())) {
			return false;
		} else if (b->depends_on(a.get())) {
			return true;
		}

		return a.get() < b.get();
	});
}

void database::process_modules_at_dir(const std::filesystem::path &path, module *parent_module)
{
	std::filesystem::directory_iterator dir_iterator(path);

	for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
		if (!dir_entry.is_directory()) {
			continue;
		}

		if (dir_entry.path().stem().string().front() == '.') {
			continue; //ignore hidden directories, e.g. ".git" dirs
		}

		const std::string module_identifier = dir_entry.path().stem().string();
		auto module = std::make_unique<metternich::module>(module_identifier, dir_entry.path(), parent_module);

		std::filesystem::path submodules_path = dir_entry.path() / "modules";
		if (std::filesystem::exists(submodules_path)) {
			this->process_modules_at_dir(submodules_path, module.get());
		}

		this->modules_by_identifier[module_identifier] = module.get();
		this->modules.push_back(std::move(module));
	}
}

std::vector<std::filesystem::path> database::get_module_paths() const
{
	std::vector<std::filesystem::path> module_paths;

	for (const std::unique_ptr<module> &module : this->modules) {
		module_paths.push_back(module->get_path());
	}

	return module_paths;
}

const std::filesystem::path &database::get_tagged_image_path(const std::map<std::string, std::filesystem::path> &image_paths_by_tag, const std::string &base_tag, const std::vector<std::vector<std::string>> &suffix_list_with_fallbacks, const std::string &final_suffix) const
{
	std::vector<std::string> suffix_combinations = string::get_suffix_combinations(suffix_list_with_fallbacks);

	for (const std::string &suffix : suffix_combinations) {
		auto find_iterator = image_paths_by_tag.find(base_tag + suffix + final_suffix);
		if (find_iterator != image_paths_by_tag.end()) {
			return find_iterator->second;
		}
	}

	throw std::runtime_error("No image found for base tag \"" + base_tag + "\".");
}

}
