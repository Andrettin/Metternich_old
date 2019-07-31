#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <string>

namespace Metternich {

class Character;
class Province;

class LandedTitle : public DataEntry, public DataType<LandedTitle>
{
	Q_OBJECT

	Q_PROPERTY(Metternich::Character* holder MEMBER Holder READ GetHolder NOTIFY HolderChanged)

public:
	LandedTitle(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "landed_title";
	static constexpr const char *DatabaseFolder = "landed_titles";
	static constexpr const char *BaronyPrefix = "b_";
	static constexpr const char *CountyPrefix = "c_";
	static constexpr const char *DuchyPrefix = "d_";
	static constexpr const char *KingdomPrefix = "k_";
	static constexpr const char *EmpirePrefix = "e_";

	static LandedTitle *Add(const std::string &identifier);

	virtual std::string GetName() const override;

	Character *GetHolder() const
	{
		return this->Holder;
	}

	Province *GetProvince() const
	{
		return this->Province;
	}

	void SetProvince(Province *province)
	{
		this->Province = province;
	}

signals:
	void HolderChanged();

private:
	Character *Holder = nullptr;
	Province *Province = nullptr;
};

}
