// Copyright (C) 2025 David Edmundson <davidedmundson@kde.org>

#include "xdgsessionmanager.h"

#include <QDebug>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingReply>

using namespace Qt::StringLiterals;

class XdgInhibitionSession : public QObject
{
    Q_OBJECT
public:
    XdgInhibitionSession();
    ~XdgInhibitionSession();
private Q_SLOTS:
    void onRequestResult(uint result, const QVariantMap &results);

private:
    QString m_requestPath;
};

XdgSessionManager *XdgSessionManager::instance()
{
    static XdgSessionManager s_instance;
    return &s_instance;
}

XdgSessionManager::XdgSessionManager()
    : QObject(nullptr)
{
    QDBusConnection::sessionBus().connect(QString(),
                                          "/org/freedesktop/portal/desktop"_L1,
                                          "org.freedesktop.portal.Inhibit"_L1,
                                          "StateChanged"_L1,
                                          this,
                                          SLOT(onXdgPortalSessionStateChanged(QDBusObjectPath, QVariantMap)));

    // wrap this all on the object above
    const QString handleToken = QStringLiteral("myToken1");
    const QString sessionHandleToken = QStringLiteral("mySession1");

    QVariantMap data;
    data[QStringLiteral("handle_token")] = handleToken;
    data[QStringLiteral("session_handle_token")] = sessionHandleToken;

    QString baseName = QDBusConnection::sessionBus().baseService();
    baseName.replace(':'_L1, QString());
    baseName.replace('.'_L1, QLatin1Char('_'));

    QString resultPath = QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2").arg(baseName, handleToken);
    qDebug() << "looking for repsonse on"_L1 << resultPath;

    QDBusConnection::sessionBus()
        .connect(QString(), resultPath, "org.freedesktop.portal.Request"_L1, "Response"_L1, this, SLOT(onXdgPortalSessionMonitorCreated(uint, QVariantMap)));

    auto message = QDBusMessage::createMethodCall("org.freedesktop.portal.Desktop"_L1,
                                                  "/org/freedesktop/portal/desktop"_L1,
                                                  "org.freedesktop.portal.Inhibit"_L1,
                                                  "CreateMonitor"_L1);

    message << QString() << data;
    auto reply = QDBusConnection::sessionBus().call(message);
}

XdgSessionManager::~XdgSessionManager() = default;

void XdgSessionManager::cancel()
{
    m_logoutCancelled = true;
    qDebug() << "cancel"_L1;
}

bool XdgSessionManager::interactionAllowed() const
{
    return true;
}

void XdgSessionManager::onXdgPortalSessionMonitorCreated(uint result, const QVariantMap &results)
{
    qDebug() << "YAY"_L1 << result << results;
    if (result != 0) {
        qWarning() << "Failed to create session monitor: error"_L1 << result;
        return;
    }
    m_monitorSessionHandle = results.value(QStringLiteral("session_handle")).toString();
}

void XdgSessionManager::onXdgPortalSessionStateChanged(const QDBusObjectPath &sessionHandle, const QVariantMap &state)
{
    if (sessionHandle.path() != m_monitorSessionHandle) {
        return;
    }
    qDebug() << sessionHandle << state;

    uint sessionState = state["session-state"_L1].toUInt();
    switch (sessionState) {
    case 1:
        qDebug() << "Session state: active"_L1;
        m_logoutLock.reset();
        break;
    case 2:
        qDebug() << "Session state: queryEnd"_L1;
        m_logoutCancelled = false;
        m_logoutLock.reset(new XdgInhibitionSession);
        ackQueryEnd();
        Q_EMIT appCommitData();
        if (!m_logoutCancelled) {
            m_logoutLock.reset();
        }
        qDebug() << "app commit done"_L1;
        break;
    case 3:
        qDebug() << "Session state: ending"_L1;
        break;
    }
}

void XdgSessionManager::ackQueryEnd()
{
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.portal.Desktop"_L1,
                                                          "/org/freedesktop/portal/desktop"_L1,
                                                          "org.freedesktop.portal.Inhibit"_L1,
                                                          "QueryEndResponse"_L1);

    message << QDBusObjectPath(m_monitorSessionHandle);
    QDBusConnection::sessionBus().send(message);
}

XdgInhibitionSession::XdgInhibitionSession()
{
    // Dave, this could use the code from Harald?

    static uint requestCounter = 0;
    // wrap this all on the object above
    const QString handleToken = QStringLiteral("QtWaylandInhibition_%1").arg(requestCounter++);

    QVariantMap data;
    data[QStringLiteral("handle_token")] = handleToken;
    // data[QStringLiteral("reason")] = QString(); // hopefully optional!!! Test in gnome

    QString baseName = QDBusConnection::sessionBus().baseService();
    baseName.replace(':'_L1, QString());
    baseName.replace('.'_L1, QLatin1Char('_'));

    m_requestPath = QStringLiteral("/org/freedesktop/portal/desktop/request/%1/%2").arg(baseName, handleToken);
    qDebug() << "looking for repsonse on"_L1 << m_requestPath;

    QDBusConnection::sessionBus()
        .connect(QString(), m_requestPath, "org.freedesktop.portal.Request"_L1, "Response"_L1, this, SLOT(onRequestResult(uint, QVariantMap)));

    auto message = QDBusMessage::createMethodCall("org.freedesktop.portal.Desktop"_L1,
                                                  "/org/freedesktop/portal/desktop"_L1,
                                                  "org.freedesktop.portal.Inhibit"_L1,
                                                  "Inhibit"_L1);
    static const u_int32_t LogoutFlag = 1;
    message << QString() << LogoutFlag << data;
    auto reply = QDBusConnection::sessionBus().call(message);
}

XdgInhibitionSession::~XdgInhibitionSession()
{
    qDebug() << "ending old inhibition session"_L1;
    auto message = QDBusMessage::createMethodCall("org.freedesktop.portal.Desktop"_L1, m_requestPath, "org.freedesktop.portal.Request"_L1, "Close"_L1);
    auto reply = QDBusConnection::sessionBus().call(message);
    qDebug() << reply.type() << reply.errorMessage();
}

void XdgInhibitionSession::onRequestResult(uint result, const QVariantMap &results)
{
    qDebug() << result << results;
}

#include "xdgsessionmanager.moc"
