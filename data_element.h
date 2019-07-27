#pragma once

#include "gsml_data.h"

#include <filesystem>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

template <typename T, typename KEY = std::string>
class DataElement
{
public:
	/**
	**	@brief	Get an instance of the class by its identifier
	**
	**	@param	identifier	The instance's identifier
	**
	**	@return	The instance if found, or null otherwise
	*/
	static inline T *Get(const KEY &identifier)
	{
		typename std::map<KEY, std::unique_ptr<T>>::const_iterator find_iterator = DataElement::InstancesByIdentifier.find(identifier);

		if (find_iterator != DataElement::InstancesByIdentifier.end()) {
			return find_iterator->second.get();
		}

		if constexpr (std::is_arithmetic_v<KEY>) {
			throw std::runtime_error("Invalid \"" + std::string(T::ClassIdentifier) + "\" instance: \"" + std::to_string(identifier) + "\".");
		} else {
			throw std::runtime_error("Invalid \"" + std::string(T::ClassIdentifier) + "\" instance: \"" + identifier + "\".");
		}
	}

	/**
	**	@brief	Gets all instances of the class
	**
	**	@return	All existing instances of the class
	*/
	static inline const std::vector<T *> &GetAll()
	{
		return DataElement::Instances;
	}

	/**
	**	@brief	Add a new instance of the class
	**
	**	@param	identifier	The instance's identifier
	**
	**	@return	The new instance
	*/
	static inline T *Add(const KEY &identifier)
	{
		if constexpr (std::is_same_v<KEY, std::string>) {
			if (identifier.empty()) {
				throw std::runtime_error("Tried to add a \"" + std::string(T::ClassIdentifier) + "\" instance with an empty string identifier.");
			}
		}

		DataElement::InstancesByIdentifier[identifier] = std::make_unique<T>(identifier);
		T *instance = DataElement::InstancesByIdentifier.find(identifier)->second.get();
		DataElement::Instances.push_back(instance);

		return instance;
	}

	/**
	**	@brief	Remove an instance of the class
	**
	**	@param	instance	The instance
	*/
	static inline void Remove(T *instance)
	{
		DataElement::InstancesByIdentifier.erase(instance->GetIdentifier());
		DataElement::Instances.erase(std::remove(DataElement::Instances.begin(), DataElement::Instances.end(), instance), DataElement::Instances.end());
	}

	/**
	**	@brief	Remove the existing class instances
	*/
	static inline void Clear()
	{
		DataElement::Instances.clear();
		DataElement::InstancesByIdentifier.clear();
	}

	/**
	**	@brief	Load the database for the data element class
	*/
	static inline void LoadDatabase()
	{
		std::filesystem::path database_path("./data/common/" + std::string(T::DatabaseFolder));

		if (!std::filesystem::exists(database_path)) {
			return;
		}

		std::filesystem::recursive_directory_iterator dir_iterator(database_path);

		for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
			if (!dir_entry.is_regular_file()) {
				continue;
			}

			GSMLData gsml_data = GSMLData::ParseFile(dir_entry.path());
			for (const GSMLData &data_entry : gsml_data.GetChildren()) {
				T *instance = nullptr;
				if constexpr (std::is_same_v<KEY, int>) {
					instance = T::Add(std::stoi(data_entry.GetTag()));
				} else {
					instance = T::Add(data_entry.GetTag());
				}

				for (const GSMLProperty &property : data_entry.GetProperties()) {
					if (!instance->ProcessGSMLProperty(property)) {
						throw std::runtime_error("Invalid " + std::string(T::ClassIdentifier) + " property: \"" + property.GetKey() + "\".");
					}
				}

				for (const GSMLData &child_data : data_entry.GetChildren()) {
					if (!instance->ProcessGSMLData(child_data)) {
						throw std::runtime_error("Invalid " + std::string(T::ClassIdentifier) + " field: \"" + child_data.GetTag() + "\".");
					}
				}
			}
		}
	}

private:
	static inline std::vector<T *> Instances;
	static inline std::map<KEY, std::unique_ptr<T>> InstancesByIdentifier;
};
