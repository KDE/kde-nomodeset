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

    QObject::connect(&item, &KStatusNotifierItem::activateRequested, [&helper] {
        if (!qApp->allWindows().isEmpty()) { // already open
            const QWindowList windows = qApp->allWindows();
            for (auto window : windows) {
                if (window->isVisible()) {
                    window->requestActivate();
                }
            }
            return;
        }

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
