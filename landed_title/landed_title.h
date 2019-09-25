#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

#include <QColor>

#include <string>
#include <vector>

namespace metternich {

class Character;
class holding;
class law;
class law_group;
class Province;
enum class LandedTitleTier : int;

class LandedTitle : public data_entry, public data_type<LandedTitle>
{
	Q_OBJECT

	Q_PROPERTY(QString titled_name READ GetTitledNameQString NOTIFY TitledNameChanged)
	Q_PROPERTY(metternich::Character* holder READ GetHolder WRITE SetHolder NOTIFY HolderChanged)
	Q_PROPERTY(metternich::LandedTitle* holder_title MEMBER HolderTitle WRITE SetHolderTitle)
	Q_PROPERTY(metternich::LandedTitle* liege_title MEMBER LiegeTitle)
	Q_PROPERTY(metternich::LandedTitle* de_jure_liege_title READ GetDeJureLiegeTitle WRITE SetDeJureLiegeTitle NOTIFY DeJureLiegeTitleChanged)
	Q_PROPERTY(metternich::LandedTitle* realm READ GetRealm NOTIFY RealmChanged)
	Q_PROPERTY(metternich::Province* capital_province MEMBER CapitalProvince READ GetCapitalProvince)
	Q_PROPERTY(QVariantList laws READ get_laws_qvariant_list)

public:
	LandedTitle(const std::string &identifier) : data_entry(identifier) {}

	static constexpr const char *class_identifier = "landed_title";
	static constexpr const char *database_folder = "landed_titles";
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

	static LandedTitle *add(const std::string &identifier);

	static const char *GetTierIdentifier(const LandedTitleTier tier);
	static const char *GetTierHolderIdentifier(const LandedTitleTier tier);

	virtual void process_gsml_dated_property(const gsml_property &property, const QDateTime &date) override;
	virtual void process_gsml_scope(const gsml_data &scope) override;
	virtual void initialize() override;
	virtual void initialize_history() override;
	virtual void check() const override;

	virtual std::string get_name() const override;
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

	metternich::holding *get_holding() const
	{
		return this->holding;
	}

	void set_holding(holding *holding);

	metternich::Province *GetProvince() const
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

	const std::vector<LandedTitle *> &get_de_jure_vassal_titles() const
	{
		return this->de_jure_vassal_titles;
	}

	void add_de_jure_vassal_title(LandedTitle *title)
	{
		this->de_jure_vassal_titles.push_back(title);
	}

	void remove_de_jure_vassal_title(LandedTitle *title)
	{
		this->de_jure_vassal_titles.erase(std::remove(this->de_jure_vassal_titles.begin(), this->de_jure_vassal_titles.end(), title), this->de_jure_vassal_titles.end());
	}

	bool IsTitular() const
	{
		//a title is not titular if it has de jure vassals, or if it is a county belonging to a province, or a barony belonging to a holding
		return this->get_de_jure_vassal_titles().empty() && this->GetProvince() == nullptr && this->get_holding() == nullptr;
	}

	metternich::Province *GetCapitalProvince() const
	{
		return this->CapitalProvince;
	}

	std::vector<law *> get_laws() const
	{
		std::vector<law *> laws;

		for (const auto &kv_pair : this->laws) {
			laws.push_back(kv_pair.second);
		}

		return laws;
	}

	QVariantList get_laws_qvariant_list() const;
	bool has_law(const law *law) const;
	Q_INVOKABLE void add_law(metternich::law *law);
	Q_INVOKABLE void remove_law(metternich::law *law);

signals:
	void TitledNameChanged();
	void HolderChanged();
	void DeJureLiegeTitleChanged();
	void RealmChanged();

private:
	QColor Color;
	LandedTitleTier Tier;
	Character *Holder = nullptr;
	metternich::holding *holding = nullptr; //this title's holding, if it is a non-titular barony
	metternich::Province *Province = nullptr; //this title's province, if it is a non-titular county
	LandedTitle *DeJureLiegeTitle = nullptr;
	std::vector<LandedTitle *> de_jure_vassal_titles;
	metternich::Province *CapitalProvince = nullptr;
	LandedTitle *HolderTitle = nullptr; //title of this title's holder; used only for initialization, and set to null afterwards
	LandedTitle *LiegeTitle = nullptr; //title of this title's holder's liege; used only for initialization, and set to null afterwards
	std::map<law_group *, law *> laws; //the laws pertaining to the title, mapped to the respective law group
};

}
