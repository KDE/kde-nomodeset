// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ApplicationWindow {
    id: root

    title: standardTitle()
    minimumWidth: Kirigami.Units.gridUnit * 22
    minimumHeight: Kirigami.Units.gridUnit * 22

    // The sub pages can't bind root title for some reason or another so we run the title through a function.
    function standardTitle() { return i18nc("@title:window", "Safe Graphics Mode") }

    pageStack.initialPage: AuthHelper.grubCfgExists ? "qrc:/DisablePage.qml" : "qrc:/InfoPage.qml"

    // // Window instances aren't Items so we need a helper to do clean state management.
    StateGroup {
        states: [
            State {
                name: "busy"
                when: AuthHelper.busy
                PropertyChanges { target: pageStack; initialPage: "qrc:/BusyPage.qml" }
            }
            , State {
                name: "disabled"
                when: AuthHelper.disabled
                PropertyChanges { target: pageStack; initialPage: "qrc:/DisabledPage.qml" }
            }
            , State {
                name: "error"
                when: AuthHelper.error !== ""
                PropertyChanges { target: pageStack; initialPage: "qrc:/ErrorPage.qml" }
            }
        ]
    }
}
