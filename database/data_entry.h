#pragma once

#include <QObject>
#include <QString>

#include <string>

namespace metternich {

class gsml_data;
class gsml_property;

/**
**	@brief	The base class for a de(serializable) but not necessarily identifiable entry to the database
*/
class data_entry_base : public QObject
{
	Q_OBJECT

public:
	virtual ~data_entry_base() {}

	virtual void process_gsml_property(const gsml_property &property);
	virtual void process_gsml_scope(const gsml_data &scope);

	virtual void process_gsml_dated_property(const gsml_property &property, const QDateTime &date)
	{
		Q_UNUSED(date)
		this->process_gsml_property(property);
	}

	virtual void process_gsml_dated_scope(const gsml_data &scope, const QDateTime &date)
	{
		Q_UNUSED(date)
		this->process_gsml_scope(scope);
	}

	void load_history(gsml_data &data);

	virtual void initialize()
	{
		this->initialized = true;
	}

	virtual void initialize_history() { }

	virtual void check() const {}

	bool is_initialized() const
	{
		return this->initialized;
	}

private:
	bool initialized = false;
};

/**
**	@brief	The base class for a de(serializable) and identifiable entry to the database
*/
class identifiable_data_entry_base : public data_entry_base
{
	Q_OBJECT

	Q_PROPERTY(QString name READ get_name_qstring NOTIFY name_changed)

public:
	virtual ~identifiable_data_entry_base() override {}

	virtual std::string get_identifier_string() const = 0;

	virtual std::string get_name() const = 0;

	QString get_name_qstring() const
	{
		return QString::fromStdString(this->get_name());
	}

	virtual gsml_data get_cache_data() const;

signals:
	void name_changed();
};

/**
**	@brief	A de(serializable) and identifiable entry to the database
*/
class data_entry : public identifiable_data_entry_base
{
	Q_OBJECT

	Q_PROPERTY(QString identifier READ get_identifier_qstring CONSTANT)

public:
	data_entry(const std::string &identifier) : identifier(identifier) {}
	virtual ~data_entry() override {}

	const std::string &get_identifier() const
	{
		return this->identifier;
	}

	QString get_identifier_qstring() const
	{
		return QString::fromStdString(this->get_identifier());
	}

	virtual std::string get_identifier_string() const override
	{
		return this->get_identifier();
	}

	virtual std::string get_name() const override;

private:
	std::string identifier;
};

/**
**	@brief	A de(serializable) and identifiable entry to the database, using a number as its identifier
*/
class numeric_data_entry : public identifiable_data_entry_base
{
	Q_OBJECT

public:
	numeric_data_entry(const int identifier) : identifier(identifier) {}
	virtual ~numeric_data_entry() override {}

	int get_identifier() const
	{
		return this->identifier;
	}

	virtual std::string get_identifier_string() const override
	{
		return std::to_string(this->get_identifier());
	}

private:
	int identifier;
};

}
