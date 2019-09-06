#pragma once

#include <QObject>
#include <QString>

#include <string>

namespace Metternich {

class gsml_data;
class GSMLProperty;

/**
**	@brief	The base class for a de(serializable) but not necessarily identifiable entry to the database
*/
class DataEntryBase : public QObject
{
	Q_OBJECT

public:
	virtual ~DataEntryBase() {}

	virtual void ProcessGSMLProperty(const GSMLProperty &property);
	virtual void ProcessGSMLScope(const gsml_data &scope);

	virtual void ProcessGSMLDatedProperty(const GSMLProperty &property, const QDateTime &date)
	{
		Q_UNUSED(date);
		this->ProcessGSMLProperty(property);
	}

	virtual void ProcessGSMLDatedScope(const gsml_data &scope, const QDateTime &date)
	{
		Q_UNUSED(date);
		this->ProcessGSMLScope(scope);
	}

	void LoadHistory(gsml_data &data);

	virtual void Initialize()
	{
		this->Initialized = true;
	}

	virtual void InitializeHistory() { }

	virtual void Check() const {}

	bool IsInitialized() const
	{
		return this->Initialized;
	}

private:
	bool Initialized = false;
};

/**
**	@brief	The base class for a de(serializable) and identifiable entry to the database
*/
class IdentifiableDataEntryBase : public DataEntryBase
{
	Q_OBJECT

	Q_PROPERTY(QString name READ GetNameQString NOTIFY NameChanged)

public:
	virtual ~IdentifiableDataEntryBase() override {}

	virtual std::string GetIdentifierString() const = 0;

	virtual std::string GetName() const = 0;

	QString GetNameQString() const
	{
		return QString::fromStdString(this->GetName());
	}

signals:
	void NameChanged();
};

/**
**	@brief	A de(serializable) and identifiable entry to the database
*/
class DataEntry : public IdentifiableDataEntryBase
{
	Q_OBJECT

	Q_PROPERTY(QString identifier READ GetIdentifierQString CONSTANT)

public:
	DataEntry(const std::string &identifier) : Identifier(identifier) {}
	virtual ~DataEntry() override {}

	const std::string &GetIdentifier() const
	{
		return this->Identifier;
	}

	QString GetIdentifierQString() const
	{
		return QString::fromStdString(this->GetIdentifier());
	}

	virtual std::string GetIdentifierString() const override
	{
		return this->GetIdentifier();
	}

	virtual std::string GetName() const override;

private:
	std::string Identifier;
};

/**
**	@brief	An de(serializable) and identifiable entry to the database, using a number as its identifier
*/
class NumericDataEntry : public IdentifiableDataEntryBase
{
	Q_OBJECT

public:
	NumericDataEntry(const int identifier) : Identifier(identifier) {}
	virtual ~NumericDataEntry() override {}

	int GetIdentifier() const
	{
		return this->Identifier;
	}

	virtual std::string GetIdentifierString() const override
	{
		return std::to_string(this->GetIdentifier());
	}

private:
	int Identifier;
};

}
