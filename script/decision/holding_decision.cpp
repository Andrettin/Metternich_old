#include "script/decision/holding_decision.h"

#include "character/character.h"
#include "game/game.h"
#include "holding/holding.h"

namespace metternich {

bool holding_decision::check_conditions(const holding *holding, const character *source) const
{
	Q_UNUSED(source)
	return scoped_decision::check_conditions(holding);
}

Q_INVOKABLE bool holding_decision::check_conditions(const QVariant &holding_variant, const QVariant &source_variant) const
{
	QObject *holding_object = qvariant_cast<QObject *>(holding_variant);
	const holding *holding = static_cast<metternich::holding *>(holding_object);

	QObject *source_object = qvariant_cast<QObject *>(source_variant);
	const character *source = static_cast<metternich::character *>(source_object);
	Q_UNUSED(source)

	return this->check_conditions(holding, source);
}

Q_INVOKABLE void holding_decision::do_effects(const QVariant &holding_variant, const QVariant &source_variant) const
{
	QObject *holding_object = qvariant_cast<QObject *>(holding_variant);
	holding *holding = static_cast<metternich::holding *>(holding_object);

	QObject *source_object = qvariant_cast<QObject *>(source_variant);
	character *source = static_cast<metternich::character *>(source_object);

	game::get()->post_order([this, holding, source]() {
		scoped_decision::do_effects(holding, source);
	});
}

Q_INVOKABLE QString holding_decision::get_effects_string(const QVariant &holding_variant) const
{
	QObject *holding_object = qvariant_cast<QObject *>(holding_variant);
	const holding *holding = static_cast<metternich::holding *>(holding_object);
	return QString::fromStdString(scoped_decision::get_effects_string(holding));
}

}
