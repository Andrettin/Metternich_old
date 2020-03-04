#pragma once

#include <QObject>
#include <QVariantList>

#include <filesystem>
#include <set>
#include <string>

namespace metternich {

class gsml_data;
class gsml_property;

class module final : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QVariantList dependencies READ get_dependencies_qvariant_list)

public:
	module(const std::string &identifier, const std::filesystem::path &path, module *parent_module)
		: identifier(identifier), path(path), parent_module(parent_module)
	{
	}

	void process_gsml_property(const gsml_property &property);
	void process_gsml_scope(const gsml_data &scope) { Q_UNUSED(scope) }

	const std::string &get_identifier() const
	{
		return this->identifier;
	}

	const std::filesystem::path &get_path() const
	{
		return this->path;
	}

	QVariantList get_dependencies_qvariant_list() const;

	Q_INVOKABLE void add_dependency(module *module)
	{
		if (module->depends_on(this)) {
			throw std::runtime_error("Cannot make module \"" + this->identifier + "\" depend on module \"" + module->identifier + "\", as that would create a circular dependency.");
		}

		this->dependencies.insert(module);
	}

	Q_INVOKABLE void remove_dependency(module *module)
	{
		this->dependencies.erase(module);
	}

	bool depends_on(module *module) const
	{
		if (module == this->parent_module) {
			return true;
		}

		if (this->dependencies.contains(module)) {
			return true;
		}

		if (this->parent_module != nullptr) {
			return this->parent_module->depends_on(module);
		}

		return false;
	}

private:
	std::string identifier;
	std::filesystem::path path; //the module's path
	module *parent_module = nullptr;
	std::set<module *> dependencies; //modules on which this one is dependent
};

}
