# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

add_definitions(-DTRANSLATION_DOMAIN=\"kde-nomodeset\")

add_executable(kde-nomodeset main.cpp qml/qml.qrc)
target_link_libraries(kde-nomodeset
    KF5::Notifications
    KF5::I18n
    KF5::Declarative
    KF5::AuthCore
)

install(TARGETS kde-nomodeset ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.nomodeset.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )

add_subdirectory(polkit)
