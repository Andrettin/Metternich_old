#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <vector>

namespace metternich {

class Condition;
class employment_type;
class holding;
class holding_type;

class Building : public data_entry, public data_type<Building>
{
	Q_OBJECT

	Q_PROPERTY(QString icon READ GetIconPathQString WRITE SetIconPathQString NOTIFY IconPathChanged)
	Q_PROPERTY(QString icon_path READ GetIconPathQString WRITE SetIconPathQString NOTIFY IconPathChanged)
	Q_PROPERTY(QVariantList holding_types READ get_holding_types_qvariant_list)
	Q_PROPERTY(int construction_days MEMBER ConstructionDays READ GetConstructionDays)
	Q_PROPERTY(metternich::employment_type* employment_type MEMBER employment_type READ get_employment_type)
	Q_PROPERTY(int workforce MEMBER Workforce READ GetWorkforce)

public:
	static constexpr const char *class_identifier = "building";
	static constexpr const char *database_folder = "buildings";

	Building(const std::string &identifier);
	virtual ~Building() override;

	virtual void process_gsml_scope(const gsml_data &scope) override;

	const std::string &GetIconPath() const
	{
		return this->IconPath;
	}

	QString GetIconPathQString() const
	{
		return QString::fromStdString(this->IconPath);
	}

	void SetIconPath(const std::string &icon_path)
	{
		if (icon_path == this->GetIconPath()) {
			return;
		}

		this->IconPath = icon_path;
		emit IconPathChanged();
	}

	void SetIconPathQString(const QString &icon_path)
	{
		this->SetIconPath(icon_path.toStdString());
	}

	const std::vector<holding_type *> &get_holding_types() const
	{
		return this->holding_types;
	}

	QVariantList get_holding_types_qvariant_list() const;
	Q_INVOKABLE void add_holding_type(holding_type *holding_type);
	Q_INVOKABLE void remove_holding_type(holding_type *holding_type);

	int GetConstructionDays() const
	{
		return this->ConstructionDays;
	}

	metternich::employment_type *get_employment_type() const
	{
		return this->employment_type;
	}

	int GetWorkforce() const
	{
		return this->Workforce;
	}

	bool is_available_for_holding(const holding *holding) const;
	bool is_buildable_in_holding(const holding *holding) const;

signals:
	void IconPathChanged();

private:
	std::string IconPath;
	std::vector<holding_type *> holding_types;
	int ConstructionDays = 0; //how many days does it take to construct this building
	std::unique_ptr<Condition> Preconditions;
	std::unique_ptr<Condition> Conditions;
	metternich::employment_type *employment_type = nullptr;
	int Workforce = 0; //how many workers does this building allow for its employment type
};

}
