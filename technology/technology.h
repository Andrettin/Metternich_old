#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class holding;
class technology_area;
class territory;
enum class technology_category;

template <typename T>
class and_condition;

template <typename T>
class condition;

template <typename T>
class modifier;

class technology final : public data_entry, public data_type<technology>
{
	Q_OBJECT

	Q_PROPERTY(metternich::technology_area* area READ get_area WRITE set_area NOTIFY area_changed)
	Q_PROPERTY(int level READ get_level CONSTANT)
	Q_PROPERTY(QString icon_tag READ get_icon_tag_qstring WRITE set_icon_tag_qstring)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)
	Q_PROPERTY(QVariantList required_technologies READ get_required_technologies_qvariant_list)
	Q_PROPERTY(QVariantList allowed_technologies READ get_allowed_technologies_qvariant_list)

public:
	static constexpr const char *class_identifier = "technology";
	static constexpr const char *database_folder = "technologies";

	static std::vector<technology *> get_all_sorted();

	technology(const std::string &identifier);
	virtual ~technology() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void check() const override;

	technology_category get_category() const;

	technology_area *get_area() const
	{
		return this->area;
	}

	void set_area(technology_area *area);

	const std::string &get_icon_tag() const
	{
		if (this->icon_tag.empty()) {
			return this->get_identifier();
		}

		return this->icon_tag;
	}

	void set_icon_tag(const std::string &icon_tag)
	{
		if (icon_tag == this->get_icon_tag()) {
			return;
		}

		this->icon_tag = icon_tag;
	}

	QString get_icon_tag_qstring() const
	{
		return QString::fromStdString(this->get_icon_tag());
	}

	void set_icon_tag_qstring(const QString &icon_tag)
	{
		this->set_icon_tag(icon_tag.toStdString());
	}

	const std::filesystem::path &get_icon_path() const;

	QString get_icon_path_qstring() const
	{
		return "file:///" + QString::fromStdString(this->get_icon_path().string());
	}

	const std::set<technology *> &get_required_technologies() const
	{
		return this->required_technologies;
	}

	QVariantList get_required_technologies_qvariant_list() const;

	bool requires_technology(technology *technology) const
	{
		if (this->required_technologies.contains(technology)) {
			return true;
		}

		for (metternich::technology *required_technology : this->get_required_technologies()) {
			if (required_technology->requires_technology(technology)) {
				return true;
			}
		}

		return false;
	}

	Q_INVOKABLE void add_required_technology(technology *technology)
	{
		this->required_technologies.insert(technology);
		technology->allowed_technologies.push_back(this);
	}

	Q_INVOKABLE void remove_required_technology(technology *technology);

	QVariantList get_allowed_technologies_qvariant_list() const;

	int get_level() const
	{
		//get the technology's level, i.e. 0 if it has no prerequisites, 1 if it has only prerequisites with no prerequisites and so forth
		int level = 0;

		for (technology *technology : this->get_required_technologies()) {
			level = std::max(level, technology->get_level() + 1);
		}

		return level;
	}

	const condition<territory> *get_preconditions() const
	{
		return this->preconditions.get();
	}

	const condition<territory> *get_conditions() const;

	const std::unique_ptr<modifier<holding>> &get_holding_modifier() const
	{
		return this->holding_modifier;
	}

	const std::unique_ptr<modifier<territory>> &get_territory_modifier() const
	{
		return this->territory_modifier;
	}

signals:
	void area_changed();

private:
	technology_area *area = nullptr;
	std::string icon_tag;
	std::set<technology *> required_technologies;
	std::vector<technology *> allowed_technologies; //technologies allowed by this one
	std::unique_ptr<condition<territory>> preconditions;
	std::unique_ptr<and_condition<territory>> conditions;
	std::unique_ptr<modifier<holding>> holding_modifier; //the modifier applied to holdings in territories with this technology
	std::unique_ptr<modifier<territory>> territory_modifier; //the modifier applied to territories with this technology
};

}
