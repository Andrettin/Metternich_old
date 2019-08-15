#include "map/province.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "engine_interface.h"
#include "game/game.h"
#include "holding/holding.h"
#include "holding/holding_type.h"
#include "landed_title/landed_title.h"
#include "map/map.h"
#include "map/region.h"
#include "population/population_unit.h"
#include "religion.h"
#include "translator.h"
#include "util.h"

#include <QPainter>

namespace Metternich {

/**
**	@brief	Get an instance of the class by the RGB value associated with it
**
**	@param	rgb	The instance's RGB
**
**	@return	The instance if found, or null otherwise
*/
Province *Province::GetByRGB(const QRgb &rgb)
{
	typename std::map<QRgb, Province *>::const_iterator find_iterator = Province::InstancesByRGB.find(rgb);

	if (find_iterator != Province::InstancesByRGB.end()) {
		return find_iterator->second;
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
		//the assignment operator sets the holding's type (creating the holding if it doesn't exist), while the addition and subtraction operators add or remove buildings in the holding
		if (property.GetOperator() == GSMLOperator::Assignment) {
			LandedTitle *barony = LandedTitle::Get(property.GetKey());
			HoldingType *holding_type = HoldingType::Get(property.GetValue());

			Holding *holding = this->GetHolding(barony);
			if (holding != nullptr) {
				holding->SetType(holding_type);
			} else {
				this->CreateHolding(barony, holding_type);
			}
		}
	} else {
		DataEntryBase::ProcessGSMLProperty(property);
	}
}

/**
**	@brief	Process GSML data scope
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
**	@brief	Initialize the province
*/
void Province::Initialize()
{
	this->PopulationUnits.clear();

	for (Holding *holding : this->GetHoldings()) {
		holding->Initialize();
	}

	this->CalculatePopulation();
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
		if (this->GetCulture() == nullptr) {
			throw std::runtime_error("Province \"" + this->GetIdentifier() + "\" has no culture.");
		}

		if (this->GetReligion() == nullptr) {
			throw std::runtime_error("Province \"" + this->GetIdentifier() + "\" has no religion.");
		}
	}
}

/**
**	@brief	Get the province's name
**
**	@return	The province's name
*/
std::string Province::GetName() const
{
	return Translator::GetInstance()->Translate(this->GetCounty()->GetIdentifier(), {this->GetCulture()->GetIdentifier(), this->GetCulture()->GetCultureGroup()->GetIdentifier(), this->GetReligion()->GetIdentifier()});
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
void Province::CreateImage(const std::set<int> &pixel_indexes)
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

	this->Image = QImage(this->Rect.size(), QImage::Format_ARGB32);
	this->Image.fill(qRgba(0, 0, 0, 0));

	for (const int index : pixel_indexes) {
		QPoint pixel_pos = Map::GetPixelPosition(index) - this->Rect.topLeft();
		this->Image.setPixelColor(pixel_pos, this->GetColor());
	}

	this->UpdateImage();
}

/**
**	@brief	Update the province's image
*/
void Province::UpdateImage()
{
	QColor province_color = this->GetCounty()->GetColor();
	const LandedTitle *realm = this->GetCounty()->GetRealm();
	if (realm != nullptr) {
		province_color = realm->GetColor();
	}

	QColor border_color = QColor(province_color.red() / 2, province_color.green() / 2, province_color.blue() / 2);

	const int pixel_count = this->Image.width() * this->Image.height();

	QRgb *rgb_data = reinterpret_cast<QRgb *>(this->Image.bits());
	for (int i = 0; i < pixel_count; ++i) {
		QRgb &pixel_rgb = rgb_data[i];

		const int pixel_alpha = qAlpha(pixel_rgb);
		if (pixel_alpha == 0) {
			continue; //only modify non-alpha pixels of the image, i.e. the pixels of the province itself
		}

		QPoint pixel_pos = IndexToPoint(i, this->Image.size());

		//check if the pixel is adjacent to one not belonging to this province
		bool border_pixel = false;

		if (pixel_pos.x() == 0 || pixel_pos.y() == 0 || pixel_pos.x() == (this->Image.width() - 1) || pixel_pos.y() == (this->Image.height() - 1)) {
			border_pixel = true;
		} else {
			for (int x_offset = -1; x_offset <= 1; ++x_offset) {
				for (int y_offset = -1; y_offset <= 1; ++y_offset) {
					if (x_offset == 0 && y_offset == 0) {
						continue;
					}

					if (x_offset != 0 && y_offset != 0) {
						continue; //don't color pixels that are only diagonally adjacent to those of other provinces
					}

					QPoint adjacent_pos = pixel_pos + QPoint(x_offset, y_offset);

					const int adjacent_alpha = qAlpha(rgb_data[PointToIndex(adjacent_pos, this->Image.size())]);

					if (adjacent_alpha == 0) {
						border_pixel = true;
						break;
					}
				}
				if (border_pixel) {
					break;
				}
			}
		}

		if (border_pixel) {
			if (this->IsSelected()) {
				//if the province is selected, and this pixel is adjacent to a pixel not belonging to this province, then highlight it
				pixel_rgb = QColor(Qt::yellow).rgba(); //set border pixels to yellow if the province is selected
			} else {
				pixel_rgb = border_color.rgba();
			}
			continue;
		}

		pixel_rgb = province_color.rgba();
	}

	emit ImageChanged();
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
**	@brief	Sets whether the province is selected
**
**	@param	selected	Whether the province is being selected
**
**	@param	notify		Whether to emit signals indicating the change
*/
void Province::SetSelected(const bool selected, const bool notify)
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

	this->UpdateImage();

	if (notify) {
		emit SelectedChanged();
		EngineInterface::GetInstance()->emit SelectedProvinceChanged();
	}
}

/**
**	@brief	Add a population unit to the province
*/
void Province::AddPopulationUnit(std::unique_ptr<PopulationUnit> &&population_unit)
{
	this->PopulationUnits.push_back(std::move(population_unit));
}

}
