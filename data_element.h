#pragma once

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

private:
	static inline std::vector<T *> Instances;
	static inline std::map<KEY, std::unique_ptr<T>> InstancesByIdentifier;
};
