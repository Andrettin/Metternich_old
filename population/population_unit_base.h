#pragma once

#include "database/data_entry.h"

namespace metternich {

class province;
class region;

class population_unit_base : public data_entry_base
{
	Q_OBJECT

	Q_PROPERTY(int size READ get_size WRITE set_size NOTIFY size_changed)
	Q_PROPERTY(metternich::province* province READ get_province WRITE set_province NOTIFY province_changed)
	Q_PROPERTY(metternich::region* region READ get_region WRITE set_region NOTIFY region_changed)
	Q_PROPERTY(bool discount_existing READ discounts_existing WRITE set_discount_existing NOTIFY discount_existing_changed)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring NOTIFY icon_path_changed)

public:
	int get_size() const
	{
		return this->size;
	}

	virtual void set_size(const int size)
	{
		if (size == this->get_size()) {
			return;
		}

		this->size = std::max(size, 0);
		emit size_changed();
	}

	void change_size(const int change)
	{
		this->set_size(this->get_size() + change);
	}

	metternich::province *get_province() const
	{
		return this->province;
	}

	void set_province(province *province)
	{
		if (province == this->get_province()) {
			return;
		}

		this->province = province;
		emit province_changed();
	}

	metternich::region *get_region() const
	{
		return this->region;
	}

	void set_region(region *region)
	{
		if (region == this->get_region()) {
			return;
		}

		this->region = region;
		emit region_changed();
	}

	bool discounts_existing() const
	{
		return this->discount_existing;
	}

	virtual void set_discount_existing(const bool discount_existing)
	{
		if (discount_existing == this->discounts_existing()) {
			return;
		}

		this->discount_existing = discount_existing;
		emit discount_existing_changed();
	}

	virtual const std::filesystem::path &get_icon_path() const = 0;

	QString get_icon_path_qstring() const
	{
		return "file:///" + QString::fromStdString(this->get_icon_path().string());
	}

signals:
	void size_changed();
	void province_changed();
	void region_changed();
	void discount_existing_changed();
	void icon_path_changed();

private:
	int size = 0; //the size of the population unit, in number of individuals
	metternich::province *province = nullptr; //the province where this population unit lives; may only be used during initialization to generate population units in settlements in the province
	metternich::region *region = nullptr; //the region where this population unit lives; used only during initialization to generate population units in settlements or provinces in the region
	bool discount_existing = false; //whether to discount the size of existing population units (in this population unit's holding, province or region) with the appropriate characteristics from that of this one
};

}
