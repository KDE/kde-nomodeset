// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.12 as Kirigami

// The cmdline contains nomodeset but it isn't set by the well known grub config hook. We can't undo this but still
// want to inform the user that graphics performance may be garbage
Kirigami.Page {
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
                text: {
                    if (AuthHelper.liveSession) {
                        // Be a bit more helpful for live sessions. The user is trying to use a distro but its not working
                        // without nomodeset but the user might not understand why that is.
                        return xi18nc("@label",
                           `If regular graphics mode is not working on this system you'll need to figure out what's wrong or ask for help in
a support forum for this operating system. Whatever is wrong needs to be resolved on the installed system before
it can perform properly.`)
                    }
                    return appWindow.fixItText
                }
            }

            QQC2.Label {
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                visible: AuthHelper.liveSession && AuthHelper.livePersistent
                text: xi18nc("@info", `<note>Safe Graphics Mode will be preserved after installation.</note>`)
            }
        }
    }

    footer: QQC2.DialogButtonBox {
        standardButtons: QQC2.DialogButtonBox.Ok
        onAccepted: LifeTimeWrapper.quit()
    }
}
