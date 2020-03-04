#pragma once

#include "database/data_entry.h"
#include "database/data_type.h"

namespace metternich {

class world_type final : public data_entry, public data_type<world_type>
{
	Q_OBJECT

	Q_PROPERTY(QString texture_tag READ get_texture_tag_qstring WRITE set_texture_tag_qstring)
	Q_PROPERTY(bool star MEMBER star READ is_star)

public:
	static constexpr const char *class_identifier = "world_type";
	static constexpr const char *database_folder = "world_types";

	world_type(const std::string &identifier) : data_entry(identifier)
	{
	}

	const std::string &get_texture_tag() const
	{
		if (this->texture_tag.empty()) {
			return this->get_identifier();
		}

		return this->texture_tag;
	}

	void set_texture_tag(const std::string &texture_tag)
	{
		if (texture_tag == this->get_texture_tag()) {
			return;
		}

		this->texture_tag = texture_tag;
	}

	QString get_texture_tag_qstring() const
	{
		return QString::fromStdString(this->get_texture_tag());
	}

	void set_texture_tag_qstring(const QString &texture_tag)
	{
		this->set_texture_tag(texture_tag.toStdString());
	}

	const std::filesystem::path &get_texture_path() const;

	bool is_star() const
	{
		return this->star;
	}

private:
	std::string texture_tag;
	bool star = false;
};

}
