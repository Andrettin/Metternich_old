#include "map/province.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "engine_interface.h"
#include "game/game.h"
#include "holding/building.h"
#include "holding/holding.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/region.h"
#include "map/terrain.h"
#include "population/population_unit.h"
#include "religion.h"
#include "script/modifier.h"
#include "translator.h"
#include "util.h"

#include <QPainter>

namespace Metternich {

/**
**	@brief	Get an instance of the class by the RGB value associated with it
**	@param	rgb	The instance's RGB
**	@param	should_find	Whether it is expected that an instance should be found (i.e. if none is, then it is an error).
**	@return	The instance if found, or null otherwise
*/
Province *Province::GetByRGB(const QRgb &rgb, const bool should_find)
{
	typename std::map<QRgb, Province *>::const_iterator find_iterator = Province::InstancesByRGB.find(rgb);

	if (find_iterator != Province::InstancesByRGB.end()) {
		return find_iterator->second;
	}

	if (should_find) {
		throw std::runtime_error("No province found for RGB value: " + std::to_string(rgb) + ".");
	}

	return nullptr;
}

/**
**	@brief	Add a new instance of the class
**
**	@param	identifier	The instance's identifier
**
**	@return	The new instance
*/
Province *Province::Add(const std::string &identifier)
{
	if (identifier.substr(0, 2) != Province::Prefix) {
		throw std::runtime_error("Invalid identifier for new province: \"" + identifier + "\". Province identifiers must begin with \"" + Province::Prefix + "\".");
	}

	return DataType<Province>::Add(identifier);
}

/**
**	@brief	Constructor
*/
Province::Province(const std::string &identifier) : DataEntry(identifier)
{
	connect(this, &Province::CultureChanged, this, &IdentifiableDataEntryBase::NameChanged);
	connect(this, &Province::ReligionChanged, this, &IdentifiableDataEntryBase::NameChanged);
	connect(Game::GetInstance(), &Game::RunningChanged, this, &Province::UpdateImage);
	connect(this, &Province::SelectedChanged, this, &Province::UpdateImage);
}

/**
**	@brief	Destructor
*/
Province::~Province()
{
}

/**
**	@brief	Process a GSML property
**
**	@param	property	The property
*/
void Province::ProcessGSMLProperty(const GSMLProperty &property)
{
	if (property.GetKey().substr(0, 2) == LandedTitle::BaronyPrefix) {
		//a property related to one of the province's holdings
		LandedTitle *barony = LandedTitle::Get(property.GetKey());
		Holding *holding = this->GetHolding(barony);
		if (property.GetOperator() == GSMLOperator::Assignment) {
			//the assignment operator sets the holding's type (creating the holding if it doesn't exist)
			HoldingType *holding_type = HoldingType::Get(property.GetValue());
			if (holding != nullptr) {
				holding->SetType(holding_type);
			} else {
				this->CreateHolding(barony, holding_type);
			}
		} else if (property.GetOperator() == GSMLOperator::Addition || property.GetOperator() == GSMLOperator::Subtraction) {
			//the addition/subtraction operators add/remove buildings to/from the holding
			Building *building = Building::Get(property.GetValue());
			if (property.GetOperator() == GSMLOperator::Addition) {
				holding->AddBuilding(building);
			} else if (property.GetOperator() == GSMLOperator::Subtraction) {
				holding->RemoveBuilding(building);
			}
		}
	} else {
		DataEntryBase::ProcessGSMLProperty(property);
	}
}

/**
**	@brief	Process a GSML scope
**
**	@param	scope	The scope
*/
void Province::ProcessGSMLScope(const GSMLData &scope)
{
	const std::string &tag = scope.GetTag();
	const std::vector<std::string> &values = scope.GetValues();

	if (tag == "color") {
		if (values.size() != 3) {
			throw std::runtime_error("The \"color\" scope for provinces needs to contain exactly three values!");
		}

		const int red = std::stoi(values.at(0));
		const int green = std::stoi(values.at(1));
		const int blue = std::stoi(values.at(2));
		this->Color.setRgb(red, green, blue);

		if (Province::InstancesByRGB.find(this->Color.rgb()) != Province::InstancesByRGB.end()) {
			throw std::runtime_error("The color set for province \"" + this->GetIdentifier() + "\" is already used by province \"" + Province::InstancesByRGB.find(this->Color.rgb())->second->GetIdentifier() + "\"");
		}

		Province::InstancesByRGB[this->Color.rgb()] = this;
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

/**
**	@brief	Process a GSML history scope
**
**	@param	scope	The scope
**	@param	date	The date of the scope change
*/
void Province::ProcessGSMLDatedScope(const GSMLData &scope, const QDateTime &date)
{
	const std::string &tag = scope.GetTag();

	if (tag.substr(0, 2) == LandedTitle::BaronyPrefix) {
		//a change to the data of one of the province's holdings

		LandedTitle *barony = LandedTitle::Get(tag);
		Holding *holding = this->GetHolding(barony);
		if (holding != nullptr) {
			for (const GSMLProperty &property : scope.GetProperties()) {
				holding->ProcessGSMLDatedProperty(property, date);
			}
		} else {
			throw std::runtime_error("Province \"" + this->GetIdentifier() + "\" has no constructed holding for barony \"" + tag + "\", while having history to change the holding's data.");
		}
	} else {
		DataEntryBase::ProcessGSMLScope(scope);
	}
}

/**
**	@brief	Initialize the province's history
*/
void Province::InitializeHistory()
{
	this->PopulationUnits.clear();

	for (Holding *holding : this->GetHoldings()) {
		holding->InitializeHistory();
	}

	this->CalculatePopulation();

	if (this->BordersRiver()) {
		this->ChangePopulationCapacityAdditiveModifier(10000); //increase population capacity if this province borders a river
	}
	if (this->IsCoastal()) {
		this->ChangePopulationCapacityAdditiveModifier(10000); //increase population capacity if this province is coastal
	}
}

/**
**	@brief	Check whether the province is in a valid state
*/
void Province::Check() const
{
	if (!this->GetColor().isValid()) {
		throw std::runtime_error("Province \"" + this->GetIdentifier() + "\" has no valid color.");
	}

	if (Game::GetInstance()->IsStarting()) {
		if (this->GetCounty() != nullptr) {
			if (this->GetCulture() == nullptr) {
				throw std::runtime_error("Province \"" + this->GetIdentifier() + "\" has no culture.");
			}

			if (this->GetReligion() == nullptr) {
				throw std::runtime_error("Province \"" + this->GetIdentifier() + "\" has no religion.");
			}

			if (this->GetCapitalHolding() != nullptr && this->GetCapitalHolding()->GetProvince() != this) {
				throw std::runtime_error("Province \"" + this->GetIdentifier() + "\"'s capital holding (\"" + this->GetCapitalHolding()->GetBarony()->GetIdentifier() + "\") belongs to another province (\"" + this->GetCapitalHolding()->GetProvince()->GetIdentifier() + "\").");
			}
		}
	}
}

/**
**	@brief	Do the province's daily actions
*/
void Province::DoDay()
{
	for (Holding *holding : this->GetHoldings()) {
		holding->DoDay();
	}
}

/**
**	@brief	Do the province's monthly actions
*/
void Province::DoMonth()
{
	for (Holding *holding : this->GetHoldings()) {
		holding->DoMonth();
	}
}

/**
**	@brief	Get the province's name
**
**	@return	The province's name
*/
std::string Province::GetName() const
{
	if (this->GetCounty() != nullptr) {
		return Translator::GetInstance()->Translate(this->GetCounty()->GetIdentifier(), {this->GetCulture()->GetIdentifier(), this->GetCulture()->GetCultureGroup()->GetIdentifier(), this->GetReligion()->GetIdentifier()});
	}

	return Translator::GetInstance()->Translate(this->GetIdentifier()); //province without a county; sea zone, river, lake or wasteland
}

/**
**	@brief	Get the province's county
**
**	@return	The province's county
*/
void Province::SetCounty(LandedTitle *county)
{
	this->County = county;
	county->SetProvince(this);
}

/**
**	@brief	Create the province's image
**
**	@param	pixel_indexes	The indexes of the province's pixels
*/
void Province::CreateImage(const std::vector<int> &pixel_indexes)
{
	QPoint start_pos(-1, -1);
	QPoint end_pos(-1, -1);

	for (const int index : pixel_indexes) {
		QPoint pixel_pos = Map::GetPixelPosition(index);
		if (start_pos.x() == -1 || pixel_pos.x() < start_pos.x()) {
			start_pos.setX(pixel_pos.x());
		}
		if (start_pos.y() == -1 || pixel_pos.y() < start_pos.y()) {
			start_pos.setY(pixel_pos.y());
		}
		if (end_pos.x() == -1 || pixel_pos.x() > end_pos.x()) {
			end_pos.setX(pixel_pos.x());
		}
		if (end_pos.y() == -1 || pixel_pos.y() > end_pos.y()) {
			end_pos.setY(pixel_pos.y());
		}
	}

	this->Rect = QRect(start_pos, end_pos);

	this->Image = QImage(this->Rect.size(), QImage::Format_Indexed8);

	//index 0 = transparency, index 1 = the main color for the province, index 2 = province border
	this->Image.setColorTable({qRgba(0, 0, 0, 0), this->GetColor().rgb(), QColor(Qt::darkGray).rgb()});
	this->Image.fill(0);

	for (const int index : pixel_indexes) {
		QPoint pixel_pos = Map::GetPixelPosition(index) - this->Rect.topLeft();
		this->Image.setPixel(pixel_pos, 1);
	}
}

/**
**	@brief	Set the border pixels for the province's image
**
**	@param	pixel_indexes	The indexes of the province's border pixels
*/
void Province::SetBorderPixels(const std::vector<int> &pixel_indexes)
{
	for (const int index : pixel_indexes) {
		QPoint pixel_pos = Map::GetPixelPosition(index) - this->Rect.topLeft();
		this->Image.setPixel(pixel_pos, 2);
	}
}

/**
**	@brief	Update the province's image
*/
void Province::UpdateImage()
{
	QColor province_color;
	if (this->GetCounty() != nullptr) {
		const LandedTitle *realm = this->GetCounty()->GetRealm();
		if (realm != nullptr) {
			province_color = realm->GetColor();
		} else {
			province_color = this->GetCounty()->GetColor();
		}
	} else if (this->GetTerrain()->IsWater()) {
		province_color = QColor("#0080ff");
	} else {
		province_color = QColor(Qt::darkGray); //wasteland
	}

	QColor border_color;
	if (this->IsSelected()) {
		//if the province is selected, highlight its border pixels
		border_color = QColor(Qt::yellow);
	} else {
		border_color = QColor(province_color.red() / 2, province_color.green() / 2, province_color.blue() / 2);
	}

	this->Image.setColor(1, province_color.rgb());
	this->Image.setColor(2, border_color.rgb());

	emit ImageChanged();
}

/**
**	@brief	Set the province's terrain
**
**	@param	terrain	The new terrain
*/
void Province::SetTerrain(Metternich::Terrain *terrain)
{
	if (terrain == this->GetTerrain()) {
		return;
	}

	const Metternich::Terrain *old_terrain = this->GetTerrain();

	if (old_terrain != nullptr && old_terrain->GetModifier() != nullptr) {
		old_terrain->GetModifier()->Remove(this);
	}

	this->Terrain = terrain;

	if (terrain != nullptr && terrain->GetModifier() != nullptr) {
		terrain->GetModifier()->Apply(this);
	}

	emit TerrainChanged();
}

/**
**	@brief	Set the province's population
**
**	@param	population	The new population size for the province
*/
void Province::SetPopulation(const int population)
{
	if (population == this->GetPopulation()) {
		return;
	}

	this->Population = population;
	emit PopulationChanged();
}

/**
**	@brief	Calculate the population size for the province
*/
void Province::CalculatePopulation()
{
	int population = 0;
	for (const Holding *holding : this->GetHoldings()) {
		population += holding->GetPopulation();
	}
	this->SetPopulation(population);
}

/**
**	@brief	Set the province's population capacity additive modifier
**
**	@param	population	The new population capacity additive modifier for the province
*/
void Province::SetPopulationCapacityAdditiveModifier(const int population_capacity_modifier)
{
	if (population_capacity_modifier == this->GetPopulationCapacityAdditiveModifier()) {
		return;
	}

	for (Holding *holding : this->GetHoldings()) {
		holding->ChangeBasePopulationCapacity(-this->GetPopulationCapacityAdditiveModifier());
	}

	this->PopulationCapacityAdditiveModifier = population_capacity_modifier;

	for (Holding *holding : this->GetHoldings()) {
		holding->ChangeBasePopulationCapacity(this->GetPopulationCapacityAdditiveModifier());
	}
}

/**
**	@brief	Set the province's population capacity modifier
**
**	@param	population	The new population capacity modifier for the province
*/
void Province::SetPopulationCapacityModifier(const int population_capacity_modifier)
{
	if (population_capacity_modifier == this->GetPopulationCapacityModifier()) {
		return;
	}

	for (Holding *holding : this->GetHoldings()) {
		holding->ChangePopulationCapacityModifier(-this->GetPopulationCapacityModifier());
	}

	this->PopulationCapacityModifier = population_capacity_modifier;

	for (Holding *holding : this->GetHoldings()) {
		holding->ChangePopulationCapacityModifier(this->GetPopulationCapacityModifier());
	}
}

/**
**	@brief	Set the province's population growth modifier
**
**	@param	population	The new population growth modifier for the province
*/
void Province::SetPopulationGrowthModifier(const int population_growth_modifier)
{
	if (population_growth_modifier == this->GetPopulationGrowthModifier()) {
		return;
	}

	for (Holding *holding : this->GetHoldings()) {
		holding->ChangeBasePopulationGrowth(-this->GetPopulationGrowthModifier());
	}

	this->PopulationGrowthModifier = population_growth_modifier;

	for (Holding *holding : this->GetHoldings()) {
		holding->ChangeBasePopulationGrowth(this->GetPopulationGrowthModifier());
	}
}

/**
**	@brief	Get the province's holdings
*/
QVariantList Province::GetHoldingsQVariantList() const
{
	return ContainerToQVariantList(this->GetHoldings());
}

/**
**	@brief	Get one of the province's holdings
**
**	@param	barony	The holding's barony
*/
Holding *Province::GetHolding(LandedTitle *barony) const
{
	auto find_iterator = this->HoldingsByBarony.find(barony);
	if (find_iterator != this->HoldingsByBarony.end()) {
		return find_iterator->second.get();
	}

	return nullptr;
}

/**
**	@brief	Create a holding in the province
**
**	@param	barony	The holding's barony
**
**	@param	type	The holding's type
*/
void Province::CreateHolding(LandedTitle *barony, HoldingType *type)
{
	auto new_holding = std::make_unique<Holding>(barony, type, this);
	this->Holdings.push_back(new_holding.get());
	this->HoldingsByBarony.insert({barony, std::move(new_holding)});
	emit HoldingsChanged();
	if (this->GetCapitalHolding() == nullptr) {
		this->SetCapitalHolding(this->Holdings.front());
	}

	if (Game::GetInstance()->IsRunning()) {
		if (new_holding->GetCommodity() == nullptr) {
			new_holding->GenerateCommodity();
		}
	}
}

/**
**	@brief	Destroy a holding in the province
**
**	@param	barony	The holding's barony
*/
void Province::DestroyHolding(LandedTitle *barony)
{
	Holding *holding = this->GetHolding(barony);
	if (holding == this->GetCapitalHolding()) {
		//if the capital holding is being destroyed, set the next holding as the capital, if any exists, or otherwise set the capital holding to null
		if (this->Holdings.size() > 1) {
			this->SetCapitalHolding(this->Holdings.at(1));
		} else {
			this->SetCapitalHolding(nullptr);
		}
	}
	this->Holdings.erase(std::remove(this->Holdings.begin(), this->Holdings.end(), holding), this->Holdings.end());
	this->HoldingsByBarony.erase(barony);
	emit HoldingsChanged();
}

/**
**	@brief	Add a population unit to the province
*/
void Province::AddPopulationUnit(std::unique_ptr<PopulationUnit> &&population_unit)
{
	this->PopulationUnits.push_back(std::move(population_unit));
}

/**
**	@brief	Get whether this province borders a water province
**
**	@return	True if the province borders a water province, or false otherwise
*/
bool Province::BordersWater() const
{
	for (const Province *border_province : this->BorderProvinces) {
		if (border_province->GetTerrain()->IsWater()) {
			return true;
		}
	}

	return false;
}

/**
**	@brief	Get whether this province borders a river province
**
**	@return	True if the province borders a river province, or false otherwise
*/
bool Province::BordersRiver() const
{
	for (const Province *border_province : this->BorderProvinces) {
		if (border_province->GetTerrain()->IsRiver()) {
			return true;
		}
	}

	return false;
}

/**
**	@brief	Get whether this province is coastal
**
**	@return	True if the province is coastal, or false otherwise
*/
bool Province::IsCoastal() const
{
	for (const Province *border_province : this->BorderProvinces) {
		if (border_province->GetTerrain()->IsOcean()) {
			return true;
		}
	}

	return false;
}

/**
**	@brief	Set whether the province is selected
**
**	@param	selected				Whether the province is being selected
**
**	@param	notify_engine_interface	Whether to emit a signal notifying the engine interface of the change
*/
void Province::SetSelected(const bool selected, const bool notify_engine_interface)
{
	if (selected == this->IsSelected()) {
		return;
	}

	if (selected) {
		if (Province::SelectedProvince != nullptr) {
			Province::SelectedProvince->SetSelected(false, false);
		}
		Province::SelectedProvince = this;
	} else {
		Province::SelectedProvince = nullptr;
	}

	this->Selected = selected;
	emit SelectedChanged();

	if (notify_engine_interface) {
		EngineInterface::GetInstance()->emit SelectedProvinceChanged();
	}
}

/**
**	@brief	Get whether the province is selectable
**
**	@return	True if the province is selectable, or false otherwise
*/
bool Province::IsSelectable() const
{
	return this->GetCounty() != nullptr;
}

}
