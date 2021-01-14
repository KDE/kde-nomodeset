// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.ScrollablePage {
    title: standardTitle()

    RowLayout {
        width: parent.width

        Kirigami.Icon {
            implicitWidth: Kirigami.Units.iconSizes.enormous
            implicitHeight: implicitWidth
            source: "video-display"
        }

        QQC2.Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: xi18nc("@info",
                `<para>This system was installed in Safe Graphics Mode (also known as 'nomodeset'). While this mode often works even when
                the graphics driver is malfunctioning it is also greatly impairing the ability of your graphics card to work as intended
                because the system likely is using a very basic fallback graphics driver.</para>
                <para>It is advised to deal with whatever is wrong with your system that you felt the need to use Safe Graphics Mode.
                This likely means either upgrading the Linux kernel or installing a graphics driver that correctly supports the graphics
                card.</para>
                <para>If you are confident that you have resolved the graphics issues you can permanently enable the default graphics mode
                again.</para>`)
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
        // One would argue that enabling is the thing to do here. So there's no explicit Close button.
        // TODO maybe have another button all the same `keep safe graphics`
    }
}
