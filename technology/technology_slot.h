#pragma once

#include <QObject>

namespace metternich {

class technology;
class territory;

template <typename T>
class condition_check;

//the technology slot for a given technology in a territory
class technology_slot final : public QObject
{
	Q_OBJECT

	Q_PROPERTY(metternich::technology* technology READ get_technology CONSTANT)
	Q_PROPERTY(QString icon_path READ get_icon_path_qstring CONSTANT)
	Q_PROPERTY(bool available READ is_available NOTIFY available_changed)
	Q_PROPERTY(bool acquirable READ is_acquirable NOTIFY acquirable_changed)
	Q_PROPERTY(bool acquired READ is_acquired NOTIFY acquired_changed)
	Q_PROPERTY(QString required_technologies_string READ get_required_technologies_string CONSTANT)
	Q_PROPERTY(QString effects_string READ get_effects_string CONSTANT)

public:
	technology_slot(technology *technology, territory *territory);
	~technology_slot();

	void initialize_history();

	technology *get_technology() const
	{
		return this->technology;
	}

	territory *get_territory() const
	{
		return this->territory;
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

	bool is_acquirable() const
	{
		return this->acquirable;
	}

	void set_acquirable(const bool acquirable)
	{
		if (acquirable == this->is_acquirable() || !this->is_available()) {
			return;
		}

		this->acquirable = acquirable;
		emit acquirable_changed();
	}

	bool is_acquired() const
	{
		return this->acquired;
	}

	void set_acquired(const bool acquired);

	void create_condition_checks();

	QString get_required_technologies_string() const;
	QString get_effects_string() const;

signals:
	void available_changed();
	void acquirable_changed();
	void acquired_changed();

private:
	technology *technology = nullptr;
	territory *territory = nullptr;
	bool available = false; //whether the technology is at all available
	bool acquirable = false; //whether the technology is acquirable
	bool acquired = false;
	std::unique_ptr<condition_check<metternich::territory>> precondition_check;
	std::unique_ptr<condition_check<metternich::territory>> condition_check;
};

}
