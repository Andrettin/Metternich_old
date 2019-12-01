#pragma once

#include <QObject>

namespace metternich {

class building;
class holding;

template <typename T>
class condition_check;

/**
**	@brief	The slot for a given building in a holding
*/
class building_slot : public QObject
{
	Q_OBJECT

	Q_PROPERTY(metternich::building* building READ get_building CONSTANT)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)
	Q_PROPERTY(bool available READ is_available NOTIFY available_changed)
	Q_PROPERTY(bool buildable READ is_buildable NOTIFY buildable_changed)
	Q_PROPERTY(bool built READ is_built NOTIFY built_changed)

public:
	building_slot(building *building, holding *holding);
	~building_slot();

	building *get_building() const
	{
		return this->building;
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

signals:
	void available_changed();
	void buildable_changed();
	void built_changed();

private:
	building *building = nullptr;
	holding *holding = nullptr; //the building slot's holding
	bool available = false; //whether building is at all available
	bool buildable = false; //whether the building is buildable
	bool built = false;
	std::unique_ptr<condition_check<metternich::holding>> precondition_check;
	std::unique_ptr<condition_check<metternich::holding>> condition_check;
};

}
