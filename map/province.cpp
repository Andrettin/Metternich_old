#include "map/province.h"

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "landed_title.h"
#include "map/map.h"

#include <QPainter>

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
**	@brief	Process a GSML property
**
**	@param	property	The property
**
**	@return	True if the property key is valid (and the operator is valid for it), or false otherwise
*/
bool Province::ProcessGSMLProperty(const GSMLProperty &property)
{
	const std::string &key = property.GetKey();
	const GSMLOperator gsml_operator = property.GetOperator();
	const std::string &value = property.GetValue();

	if (key == "county") {
		if (gsml_operator == GSMLOperator::Assignment) {
			this->County = LandedTitle::Get(value);
		}
	} else {
		return false;
	}

	return true;
}

/**
**	@brief	Process GSML data scope
**
**	@param	scope	The scope
**
**	@return	True if the scope tag is valid, or false otherwise
*/
bool Province::ProcessGSMLScope(const GSMLData &scope)
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
		return false;
	}

	return true;
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
}
