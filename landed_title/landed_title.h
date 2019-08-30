#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

#include <string>
#include <vector>

namespace Metternich {

class Character;
class Holding;
class Province;
enum class LandedTitleTier : int;

class LandedTitle : public DataEntry, public DataType<LandedTitle>
{
	Q_OBJECT

	Q_PROPERTY(QString titled_name READ GetTitledNameQString NOTIFY TitledNameChanged)
	Q_PROPERTY(Metternich::Character* holder READ GetHolder WRITE SetHolder NOTIFY HolderChanged)
	Q_PROPERTY(Metternich::LandedTitle* holder_title MEMBER HolderTitle WRITE SetHolderTitle)
	Q_PROPERTY(Metternich::LandedTitle* liege_title MEMBER LiegeTitle)
	Q_PROPERTY(Metternich::LandedTitle* de_jure_liege_title READ GetDeJureLiegeTitle WRITE SetDeJureLiegeTitle NOTIFY DeJureLiegeTitleChanged)
	Q_PROPERTY(Metternich::LandedTitle* realm READ GetRealm NOTIFY RealmChanged)
	Q_PROPERTY(Metternich::Province* capital_province MEMBER CapitalProvince READ GetCapitalProvince)

public:
	LandedTitle(const std::string &identifier) : DataEntry(identifier) {}

	static constexpr const char *ClassIdentifier = "landed_title";
	static constexpr const char *DatabaseFolder = "landed_titles";
	static constexpr const char *BaronyPrefix = "b_";
	static constexpr const char *CountyPrefix = "c_";
	static constexpr const char *DuchyPrefix = "d_";
	static constexpr const char *KingdomPrefix = "k_";
	static constexpr const char *EmpirePrefix = "e_";

	//string identifiers for landed title tiers
	static constexpr const char *BaronyIdentifier = "barony";
	static constexpr const char *CountyIdentifier = "county";
	static constexpr const char *DuchyIdentifier = "duchy";
	static constexpr const char *KingdomIdentifier = "kingdom";
	static constexpr const char *EmpireIdentifier = "empire";

	//string identifiers for landed title tier holder title names
	static constexpr const char *BaronIdentifier = "baron";
	static constexpr const char *CountIdentifier = "count";
	static constexpr const char *DukeIdentifier = "duke";
	static constexpr const char *KingIdentifier = "king";
	static constexpr const char *EmperorIdentifier = "emperor";

	static LandedTitle *Add(const std::string &identifier);

	static const char *GetTierIdentifier(const LandedTitleTier tier);
	static const char *GetTierHolderIdentifier(const LandedTitleTier tier);

	virtual void ProcessGSMLDatedProperty(const GSMLProperty &property, const QDateTime &date) override;
	virtual void ProcessGSMLScope(const GSMLData &scope) override;
	virtual void InitializeHistory() override;
	virtual void Check() const override;

	virtual std::string GetName() const override;
	std::string GetTierTitleName() const;

	std::string GetTitledName() const;

	QString GetTitledNameQString() const
	{
		return QString::fromStdString(this->GetTitledName());
	}

	std::string GetHolderTitleName() const;

	const QColor &GetColor() const
	{
		return this->Color;
	}

	LandedTitleTier GetTier() const
	{
		return this->Tier;
	}

	Character *GetHolder() const
	{
		return this->Holder;
	}

	void SetHolder(Character *character);
	void SetHolderTitle(LandedTitle *title);

	Metternich::Holding *GetHolding() const
	{
		return this->Holding;
	}

	void SetHolding(Holding *holding);

	Metternich::Province *GetProvince() const
	{
		return this->Province;
	}

	void SetProvince(Province *province)
	{
		this->Province = province;
		this->CapitalProvince = province;
	}

	LandedTitle *GetRealm() const;

	LandedTitle *GetDeJureLiegeTitle() const
	{
		return this->DeJureLiegeTitle;
	}

	void SetDeJureLiegeTitle(LandedTitle *title);

	void AddDeJureVassalTitle(LandedTitle *title)
	{
		this->DeJureVassalTitles.push_back(title);
	}

	void RemoveDeJureVassalTitle(LandedTitle *title)
	{
		this->DeJureVassalTitles.erase(std::remove(this->DeJureVassalTitles.begin(), this->DeJureVassalTitles.end(), title), this->DeJureVassalTitles.end());
	}

	bool IsTitular() const
	{
		//a title is not titular if it has de jure vassals, or if it is a county belonging to a province, or a barony belonging to a holding
		return this->DeJureVassalTitles.empty() && this->GetProvince() == nullptr && this->GetHolding() == nullptr;
	}

	Metternich::Province *GetCapitalProvince() const
	{
		return this->CapitalProvince;
	}

signals:
	void TitledNameChanged();
	void HolderChanged();
	void DeJureLiegeTitleChanged();
	void RealmChanged();

private:
	QColor Color;
	LandedTitleTier Tier;
	Character *Holder = nullptr;
	Metternich::Holding *Holding = nullptr; //this title's holding, if it is a non-titular barony
	Metternich::Province *Province = nullptr; //this title's province, if it is a non-titular county
	LandedTitle *DeJureLiegeTitle = nullptr;
	std::vector<LandedTitle *> DeJureVassalTitles;
	Metternich::Province *CapitalProvince = nullptr;
	LandedTitle *HolderTitle = nullptr; //title of this title's holder; used only for initialization, and set to null afterwards
	LandedTitle *LiegeTitle = nullptr; //title of this title's holder's liege; used only for initialization, and set to null afterwards
};

}
