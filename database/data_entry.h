#pragma once

#include <QObject>

#include <string>

namespace Metternich {

class GSMLData;
class GSMLProperty;

/**
**	@brief	The base class for a de(serializable) and identifiable entry to the database
*/
class DataEntryBase : public QObject
{
	Q_OBJECT

public:
	virtual ~DataEntryBase() {}

	void ProcessGSMLProperty(const GSMLProperty &property);
	virtual void ProcessGSMLScope(const GSMLData &scope);
};

/**
**	@brief	A de(serializable) and identifiable entry to the database
*/
class DataEntry : public DataEntryBase
{
	Q_OBJECT

	Q_PROPERTY(QString identifier READ GetIdentifierQString CONSTANT)

public:
	DataEntry(const std::string &identifier) : Identifier(identifier) {}
	virtual ~DataEntry() {}

	const std::string &GetIdentifier() const
	{
		return this->Identifier;
	}

	QString GetIdentifierQString() const
	{
		return QString::fromStdString(this->GetIdentifier());
	}

	const std::string &GetName() const
	{
		return this->Identifier;
	}

private:
	std::string Identifier;
};

/**
**	@brief	An de(serializable) and identifiable entry to the database, using a number as its identifier
*/
class NumericDataEntry : public DataEntryBase
{
	Q_OBJECT

public:
	NumericDataEntry(const int identifier) : Identifier(identifier) {}
	virtual ~NumericDataEntry() {}

	int GetIdentifier() const
	{
		return this->Identifier;
	}

private:
	int Identifier;
};

}
