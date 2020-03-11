#include "character/dynasty.h"

#include "culture/culture.h"
#include "culture/culture_group.h"
#include "culture/culture_supergroup.h"
#include "history/history.h"

namespace metternich {

dynasty *dynasty::generate(metternich::culture *culture)
{
	if (culture == nullptr) {
		throw std::runtime_error("Tried to generate a dynasty with no culture.");
	}

	const std::string identifier = dynasty::generate_identifier();
	dynasty *dynasty = dynasty::add(identifier);
	dynasty->name = culture->generate_dynasty_name();
	dynasty->culture = culture;

	return dynasty;
}

void dynasty::initialize()
{
	if (this->get_culture() != nullptr) {
		if (!this->get_culture()->is_initialized()) {
			this->get_culture()->initialize(); //ensure the culture and its culture group are initialized, so that they don't add the dynasty name upwards again during initialization
		}

		const std::string name = this->get_name();
		this->get_culture()->add_dynasty_name(name);
		this->get_culture()->get_group()->add_dynasty_name(name);
		this->get_culture()->get_supergroup()->add_dynasty_name(name);
	}

	data_entry_base::initialize();
}

}
