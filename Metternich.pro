QT += core gui quick widgets location location-private positioning
CONFIG += c++2a force_debug_info

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Refer to the documentation for the
# deprecated API to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        character/character.cpp \
        character/dynasty.cpp \
        character/enemy.cpp \
        character/item.cpp \
        character/trait.cpp \
        culture/culture.cpp \
        culture/culture_base.cpp \
        database/csv_data.cpp \
        database/data_entry.cpp \
        database/database.cpp \
        database/gsml_data.cpp \
        database/gsml_parser.cpp \
        database/gsml_property.cpp \
        database/module.cpp \
        defines.cpp \
        economy/commodity.cpp \
        economy/employee.cpp \
        economy/employment.cpp \
        economy/employment_owner.cpp \
        economy/employment_type.cpp \
        economy/trade_node.cpp \
        economy/trade_route.cpp \
        engine_interface.cpp \
        game/game.cpp \
        history/history.cpp \
        holding/building.cpp \
        holding/building_slot.cpp \
        holding/holding.cpp \
        holding/holding_slot.cpp \
        holding/holding_type.cpp \
        landed_title/landed_title.cpp \
        main.cpp \
        map/map.cpp \
        map/pathfinder.cpp \
        map/province.cpp \
        map/province_image_provider.cpp \
        map/province_profile.cpp \
        map/region.cpp \
        map/terrain_type.cpp \
        map/world.cpp \
        politics/government_type.cpp \
        population/population_type.cpp \
        population/population_unit.cpp \
        religion/religion.cpp \
        religion/religion_group.cpp \
        script/chance_factor.cpp \
        script/condition/condition.cpp \
        script/condition/has_technology_condition.cpp \
        script/effect/effect.cpp \
        script/effect/effect_list.cpp \
        script/event/event_instance.cpp \
        script/event/event_option.cpp \
        script/event/scoped_event_base.cpp \
        script/factor_modifier.cpp \
        script/modifier.cpp \
        script/modifier_effect/modifier_effect.cpp \
        species/species.cpp \
        species/wildlife_unit.cpp \
        technology/technology.cpp \
        technology/technology_set.cpp \
        third_party/maskedmousearea/maskedmousearea.cpp \
        translator.cpp \
        util/image_util.cpp \
        util/point_container.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

INCLUDEPATH += third_party

PRECOMPILED_HEADER = pch.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    character/character.h \
    character/dynasty.h \
    character/enemy.h \
    character/item.h \
    character/trait.h \
    culture/culture.h \
    culture/culture_base.h \
    culture/culture_group.h \
    database/csv_data.h \
    database/data_entry.h \
    database/data_type.h \
    database/data_type_base.h \
    database/data_type_metadata.h \
    database/database.h \
    database/gsml_data.h \
    database/gsml_data_visitor.h \
    database/gsml_element_visitor.h \
    database/gsml_operator.h \
    database/gsml_parser.h \
    database/gsml_property.h \
    database/gsml_property_visitor.h \
    database/identifiable_type.h \
    database/module.h \
    database/simple_data_type.h \
    defines.h \
    economy/commodity.h \
    economy/employee.h \
    economy/employment.h \
    economy/employment_owner.h \
    economy/employment_type.h \
    economy/trade_node.h \
    economy/trade_route.h \
    empty_image_provider.h \
    engine_interface.h \
    game/game.h \
    game/game_speed.h \
    game/tick_period.h \
    history/calendar.h \
    history/history.h \
    history/timeline.h \
    holding/building.h \
    holding/building_slot.h \
    holding/holding.h \
    holding/holding_slot.h \
    holding/holding_slot_type.h \
    holding/holding_type.h \
    landed_title/landed_title.h \
    landed_title/landed_title_tier.h \
    map/map.h \
    map/map_mode.h \
    map/pathfinder.h \
    map/province.h \
    map/province_image_provider.h \
    map/province_profile.h \
    map/region.h \
    map/terrain_type.h \
    map/world.h \
    pch.h \
    phenotype.h \
    politics/government_type.h \
    politics/government_type_group.h \
    politics/law.h \
    politics/law_group.h \
    population/population_type.h \
    population/population_unit.h \
    population/population_unit_base.h \
    qunique_ptr.h \
    random.h \
    religion/religion.h \
    religion/religion_group.h \
    script/chance_factor.h \
    script/chance_util.h \
    script/condition/alive_condition.h \
    script/condition/and_condition.h \
    script/condition/borders_water_condition.h \
    script/condition/commodity_condition.h \
    script/condition/condition.h \
    script/condition/condition_check.h \
    script/condition/condition_check_base.h \
    script/condition/culture_condition.h \
    script/condition/has_any_active_trade_route_condition.h \
    script/condition/has_any_trade_route_condition.h \
    script/condition/has_any_trade_route_land_connection_condition.h \
    script/condition/has_building_condition.h \
    script/condition/has_flag_condition.h \
    script/condition/has_item_condition.h \
    script/condition/has_law_condition.h \
    script/condition/has_technology_condition.h \
    script/condition/has_trait_condition.h \
    script/condition/holding_type_condition.h \
    script/condition/location_condition.h \
    script/condition/not_condition.h \
    script/condition/or_condition.h \
    script/condition/prowess_condition.h \
    script/condition/region_condition.h \
    script/condition/terrain_condition.h \
    script/condition/tier_de_jure_title_condition.h \
    script/condition/world_condition.h \
    script/effect/combat_effect.h \
    script/effect/effect.h \
    script/effect/effect_list.h \
    script/effect/event_effect.h \
    script/effect/flags_effect.h \
    script/effect/for_effect.h \
    script/effect/if_effect.h \
    script/effect/items_effect.h \
    script/effect/location_effect.h \
    script/effect/random_list_effect.h \
    script/effect/scripted_effect.h \
    script/effect/scripted_effect_effect.h \
    script/effect/tooltip_effect.h \
    script/effect/traits_effect.h \
    script/effect/wealth_effect.h \
    script/event/character_event.h \
    script/event/event.h \
    script/event/event_instance.h \
    script/event/event_option.h \
    script/event/event_trigger.h \
    script/event/scoped_event_base.h \
    script/factor_modifier.h \
    script/flag/scoped_flag.h \
    script/holding_modifier.h \
    script/modifier.h \
    script/modifier_effect/modifier_effect.h \
    script/modifier_effect/population_capacity_modifier_effect.h \
    script/modifier_effect/population_capacity_modifier_modifier_effect.h \
    script/modifier_effect/population_growth_modifier_effect.h \
    script/modifier_effect/prowess_modifier_effect.h \
    script/province_modifier.h \
    script/scope_util.h \
    singleton.h \
    species/species.h \
    species/wildlife_unit.h \
    technology/technology.h \
    technology/technology_category.h \
    technology/technology_set.h \
    third_party/maskedmousearea/maskedmousearea.h \
    translator.h \
    type_traits.h \
    util/container_util.h \
    util/exception_util.h \
    util/filesystem_util.h \
    util/image_util.h \
    util/location_util.h \
    util/map_util.h \
    util/number_util.h \
    util/parse_util.h \
    util/point_container.h \
    util/point_util.h \
    util/rect_util.h \
    util/set_util.h \
    util/string_util.h \
    util/vector_util.h

win32 {
    INCLUDEPATH += C:/Boost
}
