#pragma once

#include "gsml_data.h"

#include <string>

class GSMLData;
class GSMLProperty;

template <typename KEY = std::string>
class DataEntry
{
public:
	using IdentifierType = KEY;

	DataEntry(const IdentifierType &identifier) : Identifier(identifier) {}
	virtual ~DataEntry() {}

	const IdentifierType &GetIdentifier() const
	{
		return this->Identifier;
	}

	virtual bool ProcessGSMLProperty(const GSMLProperty &) { return false; }
	virtual bool ProcessGSMLScope(const GSMLData &) { return false; }

private:
	IdentifierType Identifier;
};
