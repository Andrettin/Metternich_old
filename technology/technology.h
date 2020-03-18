#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class holding;
class technology_category;
class territory;

template <typename T>
class modifier;

class technology final : public data_entry, public data_type<technology>
{
	Q_OBJECT

	Q_PROPERTY(metternich::technology_category* category MEMBER category READ get_category)
	Q_PROPERTY(QString icon_tag READ get_icon_tag_qstring WRITE set_icon_tag_qstring)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)
	Q_PROPERTY(QVariantList required_technologies READ get_required_technologies_qvariant_list)
	Q_PROPERTY(QVariantList allowed_technologies READ get_allowed_technologies_qvariant_list)
	Q_PROPERTY(int row READ get_row CONSTANT)
	Q_PROPERTY(int column MEMBER column READ get_column NOTIFY column_changed)

public:
	static constexpr const char *class_identifier = "technology";
	static constexpr const char *database_folder = "technologies";
	static constexpr int max_column = 7; //8 columns

	technology(const std::string &identifier);
	virtual ~technology() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;

	virtual void initialize() override
	{
		//assign a valid column to the technology if its column is already occupied (giving priority to technologies in order of addition to the database)
		bool column_used = true;
		while (column_used) {
			column_used = false;

			for (technology *technology : technology::get_all()) {
				if (technology == this) {
					break;
				}

				if (technology->get_row() == this->get_row() && technology->get_column() == this->get_column()) {
					column_used = true;
					this->column++;
					break;
				}
			}
		}
	}

	virtual void check() const override
	{
		this->get_icon_path(); //throws an exception if the icon isn't found

		if (this->get_column() > technology::max_column) {
			qWarning() << ("Technology \"" + this->get_identifier_qstring() + "\" has its column set to " + QString::number(this->get_column()) + " (row " + QString::number(this->get_row()) + "), but the maximum column for a technology is " + QString::number(technology::max_column) + ".");
		}
	}

	technology_category *get_category() const
	{
		return this->category;
	}

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

	const std::unique_ptr<modifier<holding>> &get_holding_modifier() const
	{
		return this->holding_modifier;
	}

	const std::unique_ptr<modifier<territory>> &get_territory_modifier() const
	{
		return this->territory_modifier;
	}

	int get_row() const
	{
		int row = 0;

		for (technology *technology : this->get_required_technologies()) {
			row = std::max(row, technology->get_row() + 1);
		}

		return row;
	}

	int get_column() const
	{
		return this->column;
	}

signals:
	void column_changed();

private:
	technology_category *category = nullptr;
	std::string icon_tag;
	std::set<technology *> required_technologies;
	std::vector<technology *> allowed_technologies; //technologies allowed by this one
	std::unique_ptr<modifier<holding>> holding_modifier; //the modifier applied to holdings in territories with this technology
	std::unique_ptr<modifier<territory>> territory_modifier; //the modifier applied to territories with this technology
	int column = 0; //the technology's column in the tech tree
};

}
