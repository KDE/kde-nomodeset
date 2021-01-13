// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

#include <QDebug>
#include <QProcess>
#include <QFile>

#include <KAuth>
#include <KLocalizedString>

class NoModeSetHelper : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    KAuth::ActionReply disable(const QVariantMap &args)
    {
        Q_UNUSED(args);
        if (!QFile::exists(m_grubCfg)) {
            auto reply = KAuth::ActionReply::HelperErrorReply();
            reply.setErrorDescription(xi18nc("@info:status", "Configuration file <filename>%1</filename> not found.", m_grubCfg));
            return reply;
        }
        if (!QFile::remove(m_grubCfg)) {
            auto reply = KAuth::ActionReply::HelperErrorReply();
            reply.setErrorDescription(xi18nc("@info:status", "Deleting the configuration file <filename>%1</filename> failed."));
            return reply;
        }

        QProcess proc;
        proc.start("/usr/sbin/update-grub2", QStringList());
        if (!proc.waitForFinished()) {
            auto reply = KAuth::ActionReply::HelperErrorReply();
            reply.setErrorDescription(i18nc("@info:status", "Updating the bootloader configuration timed out."));
            return reply;
        }
        if (proc.exitCode() != 0) {
            auto reply = KAuth::ActionReply::HelperErrorReply();
            reply.setErrorDescription(i18nc("@info:status",
                                            "Updating the bootloader configuration failed - exit code: %1", QString::number(proc.exitCode())));
            return reply;
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
    const QString m_grubCfg = QStringLiteral("/etc/default/grub.d/neon-installation-nomodeset.cfg");
};

KAUTH_HELPER_MAIN("org.kde.nomodeset", NoModeSetHelper)

#include "helper.moc"
