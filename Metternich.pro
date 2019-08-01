QT += core gui quick
CONFIG += c++17 force_debug_info

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
        character.cpp \
        culture.cpp \
        database/csv_data.cpp \
        database/data_entry.cpp \
        database/gsml_data.cpp \
        defines.cpp \
        dynasty.cpp \
        engine_interface.cpp \
        game.cpp \
        history/history.cpp \
        landed_title.cpp \
        main.cpp \
        map/map.cpp \
        map/province.cpp \
        map/province_image_provider.cpp \
        third_party/maskedmousearea/maskedmousearea.cpp \
        translator.cpp

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
    character.h \
    culture.h \
    database/csv_data.h \
    database/data_entry.h \
    database/data_type.h \
    database/gsml_data.h \
    database/gsml_operator.h \
    database/gsml_property.h \
    defines.h \
    dynasty.h \
    empty_image_provider.h \
    engine_interface.h \
    game.h \
    game_speed.h \
    history/history.h \
    holding/building.h \
    holding/holding.h \
    holding/holding_type.h \
    landed_title.h \
    landed_title_tier.h \
    map/map.h \
    map/province.h \
    map/province_image_provider.h \
    pch.h \
    random.h \
    religion.h \
    third_party/maskedmousearea/maskedmousearea.h \
    trait.h \
    translator.h \
    util.h
