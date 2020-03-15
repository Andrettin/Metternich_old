#include "script/decision/filter/decision_filter.h"

#include "script/context.h"
#include "script/decision/scoped_decision.h"
#include "util/random.h"

namespace metternich {

template <typename T>
void decision_filter<T>::do_ai_decisions(const std::vector<T *> &scopes, character *source_character) const
{
	for (const auto *decision : this->decisions) {
		if (!decision->check_source_conditions(source_character)) {
			continue;
		}

		for (T *scope : scopes) {
			if (!decision->check_conditions(scope)) {
				continue;
			}

			if (random::generate(100) >= decision->calculate_ai_chance(scope, source_character)) {
				continue;
			}

			decision->do_effects(scope, source_character);

			//check if the source conditions are still valid after applying the effects
			if (!decision->check_source_conditions(source_character)) {
				break;
			}
		}
	}
}

template class decision_filter<holding>;

}
