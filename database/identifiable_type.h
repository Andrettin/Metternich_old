#pragma once

#include "qunique_ptr.h"

#include <map>
#include <memory>
#include <stdexcept>
#include <string>

namespace metternich {

//a type with an identifier to instance database
template <typename T, bool is_qobject = false>
class identifiable_type
{
	template <typename U>
	using unique_ptr = std::conditional_t<is_qobject, qunique_ptr<U>, std::unique_ptr<U>>;

public:
	static T *get(const std::string &identifier)
	{
		T *instance = T::try_get(identifier);

		if (instance == nullptr) {
			throw std::runtime_error("Invalid " + std::string(T::class_identifier) + " instance: \"" + identifier + "\".");
		}

		return instance;
	}

	static T *try_get(const std::string &identifier)
	{
		auto find_iterator = identifiable_type::instances_by_identifier.find(identifier);
		if (find_iterator != identifiable_type::instances_by_identifier.end()) {
			return find_iterator->second.get();
		}

		return nullptr;
	}

	static T *get_or_add(const std::string &identifier)
	{
		T *instance = T::try_get(identifier);
		if (instance != nullptr) {
			return instance;
		}

		return T::add(identifier);
	}

	static bool exists(const std::string &identifier)
	{
		return identifiable_type::instances_by_identifier.contains(identifier);
	}

	static T *add(const std::string &identifier)
	{
		if (identifier.empty()) {
			throw std::runtime_error("Tried to add a " + std::string(T::class_identifier) + " instance with an empty string identifier.");
		}

		if (T::exists(identifier)) {
			throw std::runtime_error("Tried to add a " + std::string(T::class_identifier) + " instance with the already-used \"" + identifier + "\" string identifier.");
		}

		if constexpr (is_qobject) {
			identifiable_type::instances_by_identifier[identifier] = make_qunique<T>(identifier);
		} else {
			identifiable_type::instances_by_identifier[identifier] = std::make_unique<T>(identifier);
		}
		T *instance = identifiable_type::instances_by_identifier.find(identifier)->second.get();

		return instance;
	}

	static void remove(T *instance)
	{
		identifiable_type::instances_by_identifier.erase(instance->get_identifier());
	}

	static void remove(const std::string &identifier)
	{
		T::remove(T::get(identifier));
	}

	static void clear()
	{
		identifiable_type::instances_by_identifier.clear();
	}

private:
	static inline std::map<std::string, unique_ptr<T>> instances_by_identifier;
};

}
