#include "map/province.h"

#include "database/gsml_data.h"
#include "database/gsml_property.h"
#include "engine_interface.h"
#include "landed_title.h"
#include "map/map.h"
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
	const int pixel_count = this->Image.width() * this->Image.height();

	QRgb *rgb_data = reinterpret_cast<QRgb *>(this->Image.bits());
	for (int i = 0; i < pixel_count; ++i) {
		QRgb &pixel_rgb = rgb_data[i];

		const int pixel_alpha = qAlpha(pixel_rgb);
		if (pixel_alpha == 0) {
			continue; //only modify non-alpha pixels of the image, i.e. the pixels of the province itself
		}

		if (this->IsSelected()) {
			//if the province is selected, and this pixel is adjacent to a pixel not belonging to this province, then highlight it
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
				pixel_rgb = QColor(Qt::yellow).rgba(); //set border pixels to yellow if the province is selected
				continue;
			}
		}

		pixel_rgb = this->GetColor().rgba();
	}

	emit ImageChanged();
}

/**
**	@brief	Sets whether the province is selected
**
**	@param	selected	Whether the province is being selected
**
**	@param	notify		Whether to emiot signals indicating the change
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

}
