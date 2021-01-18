// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ApplicationWindow {
    id: appWindow

    title: standardTitle
    minimumWidth: Kirigami.Units.gridUnit * 22
    minimumHeight: Kirigami.Units.gridUnit * 22

    onClosing: LifeTimeWrapper.quit()

    // The sub pages can't bind root title for some reason or another so we run the title through a function.
    property string standardTitle: i18nc("@title:window", "Safe Graphics Mode")
    property string preambleText: i18nc("@label",
        `This system is running in Safe Graphics Mode (also known as 'nomodeset'). While this mode often works even when
the graphics driver is malfunctioning it is also greatly impairing the ability of your graphics card to work as intended
because the system likely is using a very basic fallback graphics driver.`)
    property string fixItText: i18nc("@label",
        `It is advised to deal with whatever is wrong with your system that you felt the need to use Safe Graphics Mode.
This likely means either upgrading the Linux kernel or installing a graphics driver that correctly supports the graphics
card. If you are unsure what to do it's probably best to ask in a support forum for this operating system.`)

    pageStack.initialPage: AuthHelper.grubCfgExists ? "qrc:/DisablePage.qml" : "qrc:/InfoPage.qml"

    // Window instances aren't Items so we need a helper to do clean state management.
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
