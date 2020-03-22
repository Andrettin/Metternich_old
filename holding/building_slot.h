#pragma once

#include <QObject>

namespace metternich {

class building;
class employment;
class employment_type;
class holding;

template <typename T>
class condition_check;

/**
**	@brief	The slot for a given building in a holding
*/
class building_slot final : public QObject
{
	Q_OBJECT

	Q_PROPERTY(metternich::building* building READ get_building CONSTANT)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)
	Q_PROPERTY(bool available READ is_available NOTIFY available_changed)
	Q_PROPERTY(bool buildable READ is_buildable NOTIFY buildable_changed)
	Q_PROPERTY(bool built READ is_built NOTIFY built_changed)
	Q_PROPERTY(int workforce READ get_workforce NOTIFY workforce_changed)
	Q_PROPERTY(int workforce_capacity READ get_workforce_capacity NOTIFY workforce_capacity_changed)
	Q_PROPERTY(QString effects_string READ get_effects_string CONSTANT)
	Q_PROPERTY(metternich::employment_type* employment_type READ get_employment_type CONSTANT)

public:
	building_slot(building *building, holding *holding);
	~building_slot();

	void initialize_history();

	building *get_building() const
	{
		return this->building;
	}

	holding *get_holding() const
	{
		return this->holding;
	}

	const std::filesystem::path &get_icon_path() const;

	QString get_icon_path_qstring() const
	{
		return "file:///" + QString::fromStdString(this->get_icon_path().string());
	}

	bool is_available() const
	{
		return this->available;
	}

	void set_available(const bool available);

	bool is_buildable() const
	{
		return this->buildable;
	}

	void set_buildable(const bool buildable)
	{
		if (buildable == this->is_buildable() || !this->is_available()) {
			return;
		}

		this->buildable = buildable;
		emit buildable_changed();
	}

	bool is_built() const
	{
		return this->built;
	}

	void set_built(const bool built);

	int get_workforce() const;
	int get_workforce_capacity() const;

	void create_condition_checks();

	QString get_effects_string() const;

	employment_type *get_employment_type() const;

signals:
	void available_changed();
	void buildable_changed();
	void built_changed();
	void workforce_changed();
	void workforce_capacity_changed();

private:
	building *building = nullptr;
	holding *holding = nullptr; //the building slot's holding
	bool available = false; //whether the building is at all available
	bool buildable = false; //whether the building is buildable
	bool built = false;
	std::unique_ptr<condition_check<metternich::holding>> precondition_check;
	std::unique_ptr<condition_check<metternich::holding>> condition_check;
	std::unique_ptr<employment> employment;
};

}
