# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

cmake_minimum_required(VERSION 3.16)

project(kde-nomodeset)
set(PROJECT_VERSION "5.20.80")

set(QT_MIN_VERSION "5.15.0")
set(KF5_MIN_VERSION "5.74")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Gui Core)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    DBusAddons
    Notifications
    I18n
    Solid
    Service
    KIO
    Auth
    Declarative
)
add_subdirectory(systemd)
add_subdirectory(src)

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
