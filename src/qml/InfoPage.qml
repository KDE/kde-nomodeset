// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.12 as Kirigami

// The cmdline contains nomodeset but it isn't set by the well known grub config hook. We can't undo this but still
// want to inform the user that graphics performance may be garbage
Kirigami.Page {
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
                        `<para>This system is running in Safe Graphics Mode (also known as 'nomodeset'). While this mode often works even when
                        the graphics driver is malfunctioning it is also greatly impairing the ability of your graphics card to work as intended
                        because your system likely is using a very basic fallback graphics driver.</para>
                        <para>It is advised to deal with whatever is wrong with your system that you felt the need to use Safe Graphics Mode.
                        This likely means either upgrading the Linux kernel or installing a graphics driver that correctly supports the graphics
                        card.</para>
                        `)
        }
    }

    footer: QQC2.DialogButtonBox {
        standardButtons: QQC2.DialogButtonBox.Ok
        onAccepted: Qt.quit()
    }
}
