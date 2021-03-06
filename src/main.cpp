// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QQmlContext>
#include <QStandardPaths>
#include <QFile>
#include <QWindow>
#include <KStatusNotifierItem>
#include <KDeclarative/KDeclarative>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KAuthAction>
#include <KAuthExecuteJob>
#include <KJob>
#include <KOSRelease>

class AuthHelper : public QObject
{
    Q_OBJECT
    // Does the grub config for nomodeset exist (if not disable() will not be able to do anything)
    Q_PROPERTY(bool grubCfgExists MEMBER m_grubCfgExists CONSTANT)
    // Is a disable() call currently running
    Q_PROPERTY(bool busy MEMBER m_busy NOTIFY busyChanged)
    // Was nomodeset disabled (via disable() call)
    Q_PROPERTY(bool disabled MEMBER m_disabled NOTIFY busyChanged)
    // Last (fatal) error. Empty when no error.
    Q_PROPERTY(QString error MEMBER m_error NOTIFY errorChanged)
    // Is system running as a live session from an ISO or the like.
    Q_PROPERTY(bool liveSession MEMBER m_liveSession CONSTANT)
    // True when installing the live session results in nomodeset as well (e.g. configured in grub)
    Q_PROPERTY(bool livePersistent MEMBER m_livePersistent CONSTANT)
public:
    using QObject::QObject;

    Q_INVOKABLE void disable()
    {
        if (m_busy) {
            return;
        }
        setBusy(true);

        KAuth::Action action(QStringLiteral("org.kde.nomodeset.disable"));
        action.setHelperId(QStringLiteral("org.kde.nomodeset"));

        qDebug() << action.isValid() << action.hasHelper() << action.helperId() << action.status();
        KAuth::ExecuteJob *job = action.execute();
        connect(job, &KJob::result, this, [job, this] {
            qDebug() << job->error() << job->errorString() << job->errorText();
            switch (static_cast<KAuth::ActionReply::Error>(job->error())) {
            case KAuth::ActionReply::NoError:
                m_disabled = true;
                writeIgnore();
                break;
            case KAuth::ActionReply::AuthorizationDeniedError:
                Q_FALLTHROUGH();
            case KAuth::ActionReply::UserCancelledError:
                // leave disabled state alone -> falls back to original page in gui
                break;
            default:
                m_error = job->errorString();
                if (m_error.isEmpty()) {
                    m_error = job->errorText();
                }
                if (m_error.isEmpty()) {
                    m_error = i18nc("@info:status", "Unknown error code: %1", QString::number(job->error()));
                }
                Q_EMIT errorChanged();
                break;
            }

            setBusy(false);
        });
        job->start();
    }

    bool shouldIgnore()
    {
        return !QStandardPaths::locate(QStandardPaths::TempLocation, ignoreMaker()).isEmpty();
    }

    void writeIgnore()
    {
        const QString path =
            QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QLatin1Char('/') + ignoreMaker();
        QFile file(path);
        file.open(QIODevice::WriteOnly);
    }

Q_SIGNALS:
    void busyChanged();
    void disablingChanged();
    void errorChanged();

private:
    static QString ignoreMaker()
    {
        return QStringLiteral(".kde-nomodeset-ignore");
    }

    // BLOCKING on helper
    bool grubCfgExists()
    {
        KAuth::Action action(QStringLiteral("org.kde.nomodeset.grubcfgexists"));
        action.setHelperId(QStringLiteral("org.kde.nomodeset"));

        qDebug() << action.isValid() << action.hasHelper() << action.helperId() << action.status();
        KAuth::ExecuteJob *job = action.execute();
        bool result = false;
        connect(job, &KJob::result, this, [job, &result] {
            qDebug() << job->error() << job->errorString() << job->errorText();
            result = (job->error() == KAuth::ActionReply::NoError);
        });
        job->exec();
        return result;
    }

    bool isLiveSession()
    {
        QFile cmdline("/proc/cmdline");
        if (!cmdline.open(QFile::ReadOnly)) {
            return false;
        }
        if (cmdline.readAll().contains(QByteArray("boot=casper"))) { // ubuntus
            return true;
        }
        return false;
    }

    void setBusy(bool busy)
    {
        if (m_busy == busy) {
            return;
        }
        m_busy = busy;
        Q_EMIT busyChanged();
    }

    const bool m_grubCfgExists = grubCfgExists();
    bool m_disabled = false;
    bool m_busy = false;
    QString m_error;
    const bool m_liveSession = isLiveSession();
    // make this a function should it cover more than neon at some point
    const bool m_livePersistent = (KOSRelease().id() == QLatin1String("neon"));
};

// QML is fairly heavy. Only load it on demand. This class wraps an entire engine's life time allowing us
// to throw it away from the qml side (effectively a glorified window hide).
class LifeTimeWrapper : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    Q_INVOKABLE void quit()
    {
        deleteLater();
    }
};

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("video-card-inactive")));
    app.setDesktopFileName(QStringLiteral("org.kde.nomodeset"));

    AuthHelper helper;
    if (helper.shouldIgnore()) {
        // nomodeset was disabled in this boot already. Don't show the notification anymore until a reboot happens.
        qDebug() << "Marker file exists - a change is pending and needs a reboot";
        return 0;
    }

    KStatusNotifierItem item;
    item.setIconByName("video-card-inactive");
    item.setToolTipIconByName("video-card-inactive");
    item.setTitle(i18nc("@title", "Safe Graphics Mode Warning"));
    item.setToolTipTitle(i18nc("@title systray tooltip title", "Safe Graphics Mode"));
    item.setToolTipSubTitle(i18nc("@info:tooltip",
                                  "The system currently runs in Safe Graphics Mode - graphics card and display performance may be impaired"));
    item.setStatus(KStatusNotifierItem::NeedsAttention);
    item.setCategory(KStatusNotifierItem::SystemServices);
    item.setStandardActionsEnabled(true);

    QObject::connect(&item, &KStatusNotifierItem::activateRequested, [&item, &helper] {
        if (!qApp->allWindows().isEmpty()) { // already open
            const QWindowList windows = qApp->allWindows();
            for (auto window : windows) {
                if (window->isVisible()) {
                    window->requestActivate();
                }
            }
            return;
        }
        // No longer needs attention but will still be active. We really want the user to fix nomodeset.
        item.setStatus(KStatusNotifierItem::Active);

        auto wrapper = new LifeTimeWrapper(&helper);
        auto engine = new QQmlApplicationEngine(wrapper);

        auto l10nContext = new KLocalizedContext(engine);
        l10nContext->setTranslationDomain(QStringLiteral(TRANSLATION_DOMAIN));

        engine->rootContext()->setContextObject(l10nContext);
        engine->rootContext()->setContextProperty("AuthHelper", &helper);
        engine->rootContext()->setContextProperty("LifeTimeWrapper", wrapper);

        KDeclarative::KDeclarative::setupEngine(engine);

        engine->load(QUrl(QStringLiteral("qrc:/main.qml")));
    });

    return app.exec();
}

#include "main.moc"
