#pragma once

#include "qunique_ptr.h"

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
	
	Q_PROPERTY(QString name MEMBER name CONSTANT)
	Q_PROPERTY(QString description MEMBER description CONSTANT)
	Q_PROPERTY(QVariantList options READ get_options CONSTANT)

public:
	event_instance(const QString &name, const QString &description, std::vector<qunique_ptr<event_option_instance>> &&options);
	~event_instance();
	
	QVariantList get_options() const;
	
private:
	QString name;
	QString description;
	std::vector<qunique_ptr<event_option_instance>> options;
};

//an instance of an event option to be shown to the player
class event_option_instance : public QObject
{
	Q_OBJECT
	
	Q_PROPERTY(QString name MEMBER name CONSTANT)
	Q_PROPERTY(QString effects_string MEMBER effects_string CONSTANT)

public:
	event_option_instance(const QString &name, const QString &effects_string, const std::function<void()> &option_effects)
		: name(name), effects_string(effects_string), option_effects(option_effects)
	{
	}

	Q_INVOKABLE void do_effects() const;

private:
	QString name;
	QString effects_string;
	std::function<void()> option_effects;
};

}
