// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include <QQmlContext>
#include <QStandardPaths>
#include <QFile>
#include <KStatusNotifierItem>
#include <KDeclarative/KDeclarative>
#include <KLocalizedContext>
#include <KLocalizedString>
#include <KAuthAction>
#include <KAuthExecuteJob>
#include <KJob>

class AuthHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool grubCfgExists MEMBER m_grubCfgExists CONSTANT)
    Q_PROPERTY(bool busy MEMBER m_busy NOTIFY busyChanged)
    Q_PROPERTY(bool disabled MEMBER m_disabled NOTIFY busyChanged)
    Q_PROPERTY(QString error MEMBER m_error NOTIFY errorChanged)
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
#warning i18nc
                    m_error = "Unknown error occurred " + QString::number(job->error());
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
            result = (job->error() == KAuth::ActionReply::NoError);
        });
        job->exec();
        return result;
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
};

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("network-card")));

    AuthHelper helper;
    if (helper.shouldIgnore()) {
        // nomodeset was disabled in this boot already. Don't show the notification anymore until a reboot happens.
        qDebug() << "Marker file exists - a change is pending and needs a reboot";
        return 0;
    }

    KStatusNotifierItem item;
    item.setIconByName("network-card");
    item.setToolTipIconByName("network-card");
    item.setTitle(i18nc("@title systray icon name", "Safe Graphics Mode Warning"));
    item.setToolTipTitle(i18nc("@title systray tooltip title", "Safe Graphics Mode"));
    item.setToolTipSubTitle(i18nc("@info:tooltip",
                                  "The system currently runs in Safe Graphics Mode - graphics card and display performance may be impaired"));
    item.setStatus(KStatusNotifierItem::NeedsAttention);
    item.setCategory(KStatusNotifierItem::SystemServices);
    item.setStandardActionsEnabled(true);

    KLocalizedContext i18nContext;
    i18nContext.setTranslationDomain(QStringLiteral(TRANSLATION_DOMAIN));

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextObject(&i18nContext);
    #warning fixme this is a bit meh as the object disappears before the engine stops so you get cannot read warning spam on exit
    engine.rootContext()->setContextProperty("AuthHelper", &helper);

    KDeclarative::KDeclarative::setupEngine(&engine);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    QObject::connect(&item, &KStatusNotifierItem::activateRequested, [&engine, &url] { engine.load(url); });
    engine.load(url);

    return app.exec();
}

#include "main.moc"
