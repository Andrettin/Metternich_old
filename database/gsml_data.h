#pragma once

#include "database/gsml_property.h"

#include <QGeoCoordinate>
#include <QGeoPolygon>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace metternich {

/**
**	@brief	Grand strategy markup language data
*/
class gsml_data
{
public:
	gsml_data(const std::string tag = std::string()) : tag(tag) {}

	static gsml_data parse_file(const std::filesystem::path &filepath);

private:
	static std::vector<std::string> parse_line(const std::string &line);
	static bool parse_escaped_character(std::string &current_string, const char c);
	static void parse_tokens(const std::vector<std::string> &tokens, gsml_data **current_gsml_data);

public:
	const std::string &get_tag() const
	{
		return this->tag;
	}

	const gsml_data *get_parent() const
	{
		return this->parent;
	}

	const std::vector<gsml_data> &get_children() const
	{
		return this->children;
	}

	const gsml_data &get_child(const std::string &tag) const
	{
		for (const gsml_data &child : this->get_children()) {
			if (child.get_tag() == tag) {
				return child;
			}
		}

		throw std::runtime_error("No child with tag \"" + tag + "\" found for this GSML data.");
	}

	bool has_child(const std::string &tag) const
	{
		for (const gsml_data &child : this->get_children()) {
			if (child.get_tag() == tag) {
				return true;
			}
		}

		return false;
	}

	void add_child(gsml_data &&child)
	{
		this->children.push_back(child);
	}

	const std::vector<gsml_property> &get_properties() const
	{
		return this->properties;
	}

	const std::vector<std::string> &get_values() const
	{
		return this->values;
	}

	void add_value(const std::string &value)
	{
		this->values.push_back(value);
	}

	void sort_children()
	{
		//sort children by tag, alphabetically
		std::sort(this->children.begin(), this->children.end(), [](gsml_data &a, gsml_data &b) {
			return a.get_tag() < b.get_tag();
		});
	}

	QGeoCoordinate to_geocoordinate() const
	{
		const double longitude = std::stod(this->get_values()[0]);
		const double latitude = std::stod(this->get_values()[1]);
		return QGeoCoordinate(latitude, longitude);
	}

	QList<QGeoCoordinate> to_geocoordinate_list() const
	{
		QList<QGeoCoordinate> coordinates;

		for (const gsml_data &coordinate_data : this->get_children()) {
			QGeoCoordinate coordinate = coordinate_data.to_geocoordinate();
			coordinates.append(std::move(coordinate));
		}

		return coordinates;
	}

	QGeoPolygon to_geopolygon() const
	{
		QList<QGeoCoordinate> coordinates = this->get_child("coordinates").to_geocoordinate_list();
		QGeoPolygon geopolygon(coordinates);

		if (this->has_child("hole_coordinates")) {
			for (const gsml_data &hole_coordinate_data : this->get_child("hole_coordinates").get_children()) {
				QList<QGeoCoordinate> hole_coordinates = hole_coordinate_data.to_geocoordinate_list();
				geopolygon.addHole(hole_coordinates);
			}
		}

		return geopolygon;
	}

	void print_to_dir(const std::filesystem::path &directory) const
	{
		std::filesystem::path filepath(directory.string() + this->get_tag() + ".txt");
		std::ofstream ofstream(filepath);
		this->print_components(ofstream);
	}

	void print(std::ofstream &ofstream, const size_t indentation, const bool new_line) const
	{
		if (new_line) {
			ofstream << std::string(indentation, '\t');
		} else {
			ofstream << " ";
		}
		if (!this->get_tag().empty()) {
			ofstream << this->get_tag() << " = ";
		}
		ofstream << "{";
		if (!this->is_minor()) {
			ofstream << "\n";
		}

		this->print_components(ofstream, indentation + 1);

		if (!this->is_minor()) {
			ofstream << std::string(indentation, '\t');
		}
		ofstream << "}";
		if (!this->is_minor()) {
			ofstream << "\n";
		}
	}

private:
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

		for (const gsml_property &property : this->get_properties()) {
			property.print(ofstream, indentation);
		}

		bool new_line = true;
		for (const gsml_data &child_data : this->get_children()) {
			child_data.print(ofstream, indentation, new_line);
			if (new_line && child_data.is_minor()) {
				new_line = false;
			}
		}

		//if the last child data was minor and did not print a new line, print one now
		if (!new_line) {
			ofstream << "\n";
		}
	}

	bool is_minor() const
	{
		//get whether this is minor GSML data, e.g. just containing a few simple values
		return this->get_tag().empty() && this->get_properties().empty() && this->get_children().empty() && this->get_values().size() < 5;
	}

private:
	std::string tag;
	gsml_data *parent = nullptr;
	std::vector<gsml_data> children;
	std::vector<gsml_property> properties;
	std::vector<std::string> values; //values directly attached to the GSML data scope, used for e.g. name arrays
};

}
