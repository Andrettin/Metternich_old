#pragma once

#include "database/gsml_data_visitor.h"
#include "database/gsml_element_visitor.h"
#include "database/gsml_property.h"
#include "database/gsml_property_visitor.h"

#include <QGeoCoordinate>
#include <QGeoPath>
#include <QGeoPolygon>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <variant>
#include <vector>

namespace metternich {

class gsml_parser;

/**
**	@brief	Grand strategy markup language data
*/
class gsml_data
{
public:
	template <typename point_type>
	static gsml_data from_point(const point_type &point, const std::string &tag = std::string())
	{
		gsml_data point_data(tag);
		point_data.add_value(std::to_string(point.x()));
		point_data.add_value(std::to_string(point.y()));
		return point_data;
	}

	template <int precision>
	static gsml_data from_geocoordinate(const double longitude, const double latitude, const std::string &tag = std::string())
	{
		gsml_data geocoordinate_data(tag);

		std::ostringstream lon_string_stream;
		lon_string_stream << std::setprecision(precision) << longitude;
		geocoordinate_data.add_value(lon_string_stream.str());

		std::ostringstream lat_string_stream;
		lat_string_stream << std::setprecision(precision) << latitude;
		geocoordinate_data.add_value(lat_string_stream.str());

		return geocoordinate_data;
	}

	template <int precision>
	static gsml_data from_geocoordinate(const QGeoCoordinate &geocoordinate, const std::string &tag = std::string())
	{
		return gsml_data::from_geocoordinate<precision>(geocoordinate.longitude(), geocoordinate.latitude(), tag);
	}

	gsml_data(std::string &&tag = std::string());

	gsml_data(std::string &&tag, const gsml_operator scope_operator)
		: tag(std::move(tag)), scope_operator(scope_operator)
	{
	}

	gsml_data(const std::string &tag) : gsml_data(std::string(tag))
	{
	}

	gsml_data(const std::string &tag, const gsml_operator scope_operator)
		: gsml_data(std::string(tag), scope_operator)
	{
	}

	const std::string &get_tag() const
	{
		return this->tag;
	}

	gsml_operator get_operator() const
	{
		return this->scope_operator;
	}

	const gsml_data *get_parent() const
	{
		return this->parent;
	}

	const gsml_data &get_child(const std::string &tag) const
	{
		for (const auto &element : this->get_elements()) {
			if (!std::holds_alternative<gsml_data>(element)) {
				continue;
			}

			const gsml_data &child = std::get<gsml_data>(element);
			if (child.get_tag() == tag) {
				return child;
			}
		}

		throw std::runtime_error("No child with tag \"" + tag + "\" found for GSML data.");
	}

	bool has_child(const std::string &tag) const
	{
		for (const auto &element : this->get_elements()) {
			if (!std::holds_alternative<gsml_data>(element)) {
				continue;
			}

			if (std::get<gsml_data>(element).get_tag() == tag) {
				return true;
			}
		}

		return false;
	}

	gsml_data &add_child()
	{
		this->elements.push_back(gsml_data());
		return std::get<gsml_data>(this->elements.back());
	}

	void add_child(gsml_data &&child)
	{
		this->elements.emplace_back(std::move(child));
	}

	gsml_data &add_child(std::string &&tag, const gsml_operator gsml_operator)
	{
		this->elements.push_back(gsml_data(std::move(tag), gsml_operator));
		return std::get<gsml_data>(this->elements.back());
	}

	template <typename function_type>
	void for_each_child(const function_type &function) const
	{
		const gsml_data_visitor visitor(function);
		for (const auto &element : this->get_elements()) {
			std::visit(visitor, element);
		}
	}

	std::vector<const gsml_property *> try_get_properties(const std::string &key) const
	{
		std::vector<const gsml_property *> properties;

		this->for_each_property([&](const gsml_property &property) {
			if (property.get_key() == key) {
				properties.push_back(&property);
			}
		});

		return properties;
	}

	const std::string &get_property_value(const std::string &key) const
	{
		for (const auto &element : this->get_elements()) {
			if (!std::holds_alternative<gsml_property>(element)) {
				continue;
			}

			const gsml_property &property = std::get<gsml_property>(element);
			if (property.get_key() == key) {
				return property.get_value();
			}
		}

		throw std::runtime_error("No property with key \"" + key + "\" found for GSML data.");
	}

	void add_property(const std::string &key, const std::string &value);
	void add_property(std::string &&key, const gsml_operator gsml_operator, std::string &&value);

	template <typename function_type>
	void for_each_property(const function_type &function) const
	{
		const gsml_property_visitor visitor(function);
		for (const auto &element : this->get_elements()) {
			std::visit(visitor, element);
		}
	}

	const std::vector<std::string> &get_values() const
	{
		return this->values;
	}

	void add_value(const std::string &value)
	{
		this->values.push_back(value);
	}

	void add_value(std::string &&value)
	{
		this->values.push_back(std::move(value));
	}

	bool is_empty() const
	{
		return this->get_elements().empty() && this->get_values().empty();
	}

	const std::vector<std::variant<gsml_property, gsml_data>> &get_elements() const
	{
		return this->elements;
	}

	template <typename property_function_type, typename data_function_type>
	void for_each_element(const property_function_type &property_function, const data_function_type &data_function) const
	{
		const gsml_element_visitor visitor(property_function, data_function);
		for (const auto &element : this->get_elements()) {
			std::visit(visitor, element);
		}
	}

	QColor to_color() const
	{
		if (this->get_values().size() != 3) {
			throw std::runtime_error("Color scopes need to contain exactly three values.");
		}

		const int red = std::stoi(this->values.at(0));
		const int green = std::stoi(this->values.at(1));
		const int blue = std::stoi(this->values.at(2));

		return QColor(red, green, blue);
	}

	QPoint to_point() const
	{
		if (this->get_values().size() != 2) {
			throw std::runtime_error("Point scopes need to contain exactly two values.");
		}

		const int x = std::stoi(this->get_values()[0]);
		const int y = std::stoi(this->get_values()[1]);
		return QPoint(x, y);
	}

	QPointF to_pointf() const
	{
		if (this->get_values().size() != 2) {
			throw std::runtime_error("Point scopes need to contain exactly two values.");
		}

		const double x = std::stod(this->get_values()[0]);
		const double y = std::stod(this->get_values()[1]);
		return QPointF(x, y);
	}

	QGeoCoordinate to_geocoordinate() const
	{
		if (this->get_values().size() != 2) {
			throw std::runtime_error("Geocoordinate scopes need to contain exactly two values.");
		}

		const double longitude = std::stod(this->get_values()[0]);
		const double latitude = std::stod(this->get_values()[1]);
		return QGeoCoordinate(latitude, longitude);
	}

	QList<QGeoCoordinate> to_geocoordinate_list() const
	{
		QList<QGeoCoordinate> coordinates;

		this->for_each_child([&](const gsml_data &coordinate_data) {
			QGeoCoordinate coordinate = coordinate_data.to_geocoordinate();

			if (!coordinates.empty() && coordinate == coordinates.back()) {
				return; //don't add the coordinate if it is the same as the previous one
			}

			coordinates.append(std::move(coordinate));
		});

		return coordinates;
	}

	QGeoPolygon to_geopolygon() const
	{
		QList<QGeoCoordinate> coordinates = this->get_child("coordinates").to_geocoordinate_list();
		QGeoPolygon geopolygon(coordinates);

		if (this->has_child("hole_coordinates")) {
			this->get_child("hole_coordinates").for_each_child([&](const gsml_data &hole_coordinate_data) {
				QList<QGeoCoordinate> hole_coordinates = hole_coordinate_data.to_geocoordinate_list();
				geopolygon.addHole(hole_coordinates);
			});
		}

		return geopolygon;
	}

	QGeoPath to_geopath() const
	{
		QList<QGeoCoordinate> coordinates = this->get_child("coordinates").to_geocoordinate_list();
		QGeoPath geopath(coordinates);

		return geopath;
	}

	void print_to_dir(const std::filesystem::path &directory) const
	{
		std::filesystem::path filepath(directory / (this->get_tag() + ".txt"));
		std::ofstream ofstream(filepath);
		this->print_components(ofstream);
	}

	void print(std::ofstream &ofstream, const size_t indentation, const bool new_line) const;

	void print_components(std::ofstream &ofstream, const size_t indentation = 0) const
	{
		if (!this->get_values().empty()) {
			if (this->is_minor()) {
				ofstream << " ";
			} else {
				ofstream << std::string(indentation, '\t');
			}
		}
		for (const std::string &value : this->get_values()) {
			ofstream << value << " ";
		}
		if (!this->get_values().empty()) {
			if (!this->is_minor()) {
				ofstream << "\n";
			}
		}

		this->for_each_property([&](const gsml_property &property) {
			property.print(ofstream, indentation);
		});

		bool new_line = true;
		this->for_each_child([&](const gsml_data &child_data) {
			child_data.print(ofstream, indentation, new_line);
			if (new_line && child_data.is_minor()) {
				new_line = false;
			}
		});

		//if the last child data was minor and did not print a new line, print one now
		if (!new_line) {
			ofstream << "\n";
		}
	}

private:
	bool is_minor() const
	{
		//get whether this is minor GSML data, e.g. just containing a few simple values
		return this->get_tag().empty() && this->get_elements().empty() && this->get_values().size() < 5;
	}

private:
	std::string tag;
	gsml_operator scope_operator;
	gsml_data *parent = nullptr;
	std::vector<std::string> values; //values directly attached to the GSML data scope, used for e.g. name arrays
	std::vector<std::variant<gsml_property, gsml_data>> elements;

	friend gsml_parser;
};

}
