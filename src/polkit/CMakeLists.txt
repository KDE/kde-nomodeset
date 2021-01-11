# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

kauth_install_actions(org.kde.nomodeset org.kde.nomodeset.actions)
add_executable(kde-nomodeset-helper helper.cpp)
target_link_libraries(kde-nomodeset-helper KF5::AuthCore)

kauth_install_helper_files(kde-nomodeset-helper org.kde.nomodeset root)
install(TARGETS kde-nomodeset-helper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})
