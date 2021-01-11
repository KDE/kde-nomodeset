// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

#include <QDebug>
#include <QProcess>
#include <QFile>

#include <KAuth>

class NoModeSetHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    KAuth::ActionReply disable(const QVariantMap &args)
    {
        Q_UNUSED(args);
        #warning add error contexts
        if (!QFile::exists(m_grubCfg)) {
            qDebug() << "no config";
            return KAuth::ActionReply::HelperErrorReply();
        }
        if (!QFile::remove(m_grubCfg)) {
            qDebug() << "remove failed";
            return KAuth::ActionReply::HelperErrorReply();
        }
        QProcess proc;
        proc.start("/usr/sbin/update-grub2", QStringList());
        if (!proc.waitForFinished() || proc.exitCode() != 0) {
            qDebug() << "update failed";
            return KAuth::ActionReply::HelperErrorReply();
        }
        return KAuth::ActionReply::SuccessReply();
    }

    KAuth::ActionReply grubcfgexists(const QVariantMap &args)
    {
        Q_UNUSED(args);
        if (!QFile::exists(m_grubCfg)) {
            return KAuth::ActionReply::HelperErrorReply();
        }
        return KAuth::ActionReply::SuccessReply();
    }

private:
    QString m_grubCfg = QStringLiteral("/etc/default/grub.d/neon-installation-nomodeset.cfg");
};

KAUTH_HELPER_MAIN("org.kde.nomodeset", NoModeSetHelper)

#include "helper.moc"
