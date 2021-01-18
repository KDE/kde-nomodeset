// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ScrollablePage {
    title: appWindow.standardTitle

    RowLayout {
        width: parent.width

        Kirigami.Icon {
            implicitWidth: Kirigami.Units.iconSizes.enormous
            implicitHeight: implicitWidth
            source: "video-display"
        }

        ColumnLayout {
            Layout.fillWidth: true

            QQC2.Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                text: appWindow.preambleText
            }

            QQC2.Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                textFormat: Text.RichText
                text: appWindow.fixItText
            }

            QQC2.Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                text: i18nc("@label",
                    "If you are confident that you have resolved the graphics issues you can permanently enable the default graphics mode again.")
            }
        }
    }

    // TODO auto-nomodeset again?? ie. if things fail without nomodeset turn it back on again
    // .... when disabling nomodeset write a state file somewhere
    // .... in a marker.service write another file on boot before X is up
    // .... remove the state file and marker via plasma autostart or some other hook system that acts when X is up
    // .... in marker.service if the marker file already exists revert back to nomodeset and reboot
    //      (i.e. with a pre-existing marker the previous boot failed to bring up X suggesting whatever was broken still is)

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            action: Kirigami.Action {
                text: i18nc("@action:button", "Enable Default Graphics Mode")
                iconName: "dialog-scripts"
                onTriggered: AuthHelper.disable()
            }
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
        }
        QQC2.Button {
            action: Kirigami.Action {
                text: i18nc("@action:button", "Keep Safe Graphics Mode")
                iconName: "dialog-cancel"
                onTriggered: LifeTimeWrapper.quit()
            }
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
        }
    }
}
