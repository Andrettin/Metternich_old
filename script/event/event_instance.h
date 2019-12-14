#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

#include <functional>
#include <memory>

namespace metternich {

class event_option_instance;
	
//an instance of an event to be shown to the player
class event_instance : public QObject
{
	Q_OBJECT
	
	Q_PROPERTY(QVariantList options READ get_options CONSTANT)

public:
	event_instance(std::vector<std::unique_ptr<event_option_instance>> &&options);
	~event_instance();
	
	QVariantList get_options() const;
	
private:
	std::vector<std::unique_ptr<event_option_instance>> options;
};

//an instance of an event option to be shown to the player
class event_option_instance : public QObject
{
	Q_OBJECT
	
	Q_PROPERTY(QString name MEMBER name CONSTANT)

public:
	event_option_instance(const QString &name, const std::function<void()> &option_effects) : name(name), option_effects(option_effects)
	{
	}

	Q_INVOKABLE void do_effects() const;

private:
	QString name;
	std::function<void()> option_effects;
};

}
