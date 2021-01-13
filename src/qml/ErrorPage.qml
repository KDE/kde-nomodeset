// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.Page {
    title: i18nc("@title", "Unexpected Error")


    RowLayout {
        width: parent.width

        Kirigami.Icon {
            implicitWidth: Kirigami.Units.iconSizes.enormous
            implicitHeight: implicitWidth
            source: "dialog-error"
        }

        QQC2.Label {
            Layout.fillWidth: true
            wrapMode: Text.Wrap
            text: AuthHelper.error
        }
    }

    footer: QQC2.DialogButtonBox {
        standardButtons: QQC2.DialogButtonBox.Close
        onRejected: Qt.quit()
    }
}
