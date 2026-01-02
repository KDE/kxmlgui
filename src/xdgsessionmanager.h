#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <memory>

class QDBusObjectPath;
class KSystemInhibitor;

class XdgSessionManager : public QObject
{
    Q_OBJECT
public:
    static XdgSessionManager *instance();
    ~XdgSessionManager();

    void cancel();
    bool interactionAllowed() const;

Q_SIGNALS:
    void appCommitData();

private Q_SLOTS:
    void onXdgPortalSessionMonitorCreated(uint result, const QVariantMap &results);
    void onXdgPortalSessionStateChanged(const QDBusObjectPath &sessionHandle, const QVariantMap &state);

private:
    XdgSessionManager();
    void ackQueryEnd();

    bool m_logoutCancelled = false;
    std::unique_ptr<KSystemInhibitor> m_logoutLock;
    QString m_monitorSessionHandle;
};
