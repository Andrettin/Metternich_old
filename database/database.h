#pragma once

#include "database/gsml_data.h"
#include "singleton.h"
#include "type_traits.h"

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace metternich {

class data_entry;
class data_type_metadata;

/**
**	@brief	The database
*/
class database : public singleton<database>
{
public:
	template <typename T>
	static void process_gsml_data(T *instance, const gsml_data &data)
	{
		for (const gsml_property &property : data.get_properties()) {
			instance->process_gsml_property(property);
		}

		for (const gsml_data &child_data : data.get_children()) {
			instance->process_gsml_scope(child_data);
		}
	}

	template <typename T>
	static void process_gsml_data(T &instance, const gsml_data &data)
	{
		if constexpr (is_specialization_of_v<T, std::unique_ptr>) {
			database::process_gsml_data(instance.get(), data);
		} else {
			database::process_gsml_data(&instance, data);
		}
	}

	static void process_gsml_property_for_object(QObject *object, const gsml_property &property);

public:
	database();
	~database();

	void load();
	void initialize_history();
	void register_metadata(std::unique_ptr<data_type_metadata> &&metadata);

private:
	std::vector<std::unique_ptr<data_type_metadata>> metadata;
};

}
