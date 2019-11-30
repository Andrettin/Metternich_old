#pragma once

#include <QCryptographicHash>
#include <QString>

#include <filesystem>

namespace metternich::filesystem {

/**
**	@brief	Add the files in a directory to a checksum
**
**	@param	hash		The checksum hash
**	@param	dir_path	The path to the directory
*/
inline void add_files_to_checksum(QCryptographicHash &hash, const std::filesystem::path &dir_path)
{
	std::filesystem::recursive_directory_iterator dir_iterator(dir_path);

	for (const std::filesystem::directory_entry &dir_entry : dir_iterator) {
		if (!dir_entry.is_regular_file()) {
			continue;
		}

		QFile file(QString::fromStdString(dir_entry.path().string()));

		if (!file.open(QIODevice::ReadOnly)) {
			throw std::runtime_error("Failed to open file: " + dir_entry.path().string() + ".");
		}

		if (!hash.addData(&file)) {
			throw std::runtime_error("Failed to hash file: " + dir_entry.path().string() + ".");
		}

		file.close();
	}
}

}
