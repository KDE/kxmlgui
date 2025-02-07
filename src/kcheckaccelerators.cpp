/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
    SPDX-FileCopyrightText: 1998, 1999, 2000 KDE Team
    SPDX-FileCopyrightText: 2008 Nick Shaforostoff <shaforostoff@kde.ru>

    SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kcheckaccelerators.h"

#include <QAction>
#include <QApplication>
#include <QChar>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFile>
#include <QGroupBox>
#include <QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPushButton>
#include <QShortcutEvent>
#include <QTabBar>
#include <QTextBrowser>
#include <QVBoxLayout>

#include <KAcceleratorManager>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

KCheckAccelerators::KCheckAccelerators(QObject *parent, int key_, bool autoCheck_)
    : QObject(parent)
    , key(key_)
    , block(false)
    , autoCheck(autoCheck_)
    , drklash(nullptr)
{
    setObjectName(QStringLiteral("kapp_accel_filter"));

    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("Development"));
    alwaysShow = cg.readEntry("AlwaysShowCheckAccelerators", false);

    parent->installEventFilter(this);
    connect(&autoCheckTimer, &QTimer::timeout, this, &KCheckAccelerators::autoCheckSlot);
}

bool KCheckAccelerators::eventFilter(QObject * /*obj*/, QEvent *e)
{
    if (block) {
        return false;
    }

    switch (e->type()) { // just simplify debuggin
    case QEvent::ShortcutOverride:
        if (key && (static_cast<QKeyEvent *>(e)->key() == key)) {
            block = true;
            checkAccelerators(false);
            block = false;
            e->accept();
            return true;
        }
        break;
    case QEvent::ChildAdded:
    case QEvent::ChildRemoved:
        // Only care about widgets; this also avoids starting the timer in other
        // threads
        if (!static_cast<QChildEvent *>(e)->child()->isWidgetType()) {
            break;
        }
        Q_FALLTHROUGH();
    // fall-through
    case QEvent::Resize:
    case QEvent::LayoutRequest:
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        if (autoCheck) {
            autoCheckTimer.setSingleShot(true);
            autoCheckTimer.start(20); // 20 ms
        }
        return false;
    case QEvent::Timer:
    case QEvent::MouseMove:
    case QEvent::Paint:
        return false;
    default:
        // qCDebug(DEBUG_KXMLGUI) << "KCheckAccelerators::eventFilter " << e->type()
        // << " " << autoCheck;
        break;
    }
    return false;
}

void KCheckAccelerators::autoCheckSlot()
{
    if (block) {
        autoCheckTimer.setSingleShot(true);
        autoCheckTimer.start(20);
        return;
    }
    block = true;
    checkAccelerators(!alwaysShow);
    block = false;
}

void KCheckAccelerators::createDialog(QWidget *actWin, bool automatic)
{
    if (drklash) {
        return;
    }

    drklash = new QDialog(actWin);
    drklash->setAttribute(Qt::WA_DeleteOnClose);
    drklash->setObjectName(QStringLiteral("kapp_accel_check_dlg"));
    drklash->setWindowTitle(i18nc("@title:window", "Dr. Klash' Accelerator Diagnosis"));
    drklash->resize(500, 460);
    QVBoxLayout *layout = new QVBoxLayout(drklash);
    drklash_view = new QTextBrowser(drklash);
    layout->addWidget(drklash_view);
    QCheckBox *disableAutoCheck = nullptr;
    if (automatic) {
        disableAutoCheck = new QCheckBox(i18nc("@option:check", "Disable automatic checking"), drklash);
        connect(disableAutoCheck, &QCheckBox::toggled, this, &KCheckAccelerators::slotDisableCheck);
        layout->addWidget(disableAutoCheck);
    }
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, drklash);
    layout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::rejected, drklash, &QDialog::close);
    if (disableAutoCheck) {
        disableAutoCheck->setFocus();
    } else {
        drklash_view->setFocus();
    }
}

void KCheckAccelerators::slotDisableCheck(bool on)
{
    autoCheck = !on;
    if (!on) {
        autoCheckSlot();
    }
}

void KCheckAccelerators::checkAccelerators(bool automatic)
{
    QWidget *actWin = qApp->activeWindow();
    if (!actWin) {
        return;
    }

    KAcceleratorManager::manage(actWin);
    QString a;
    QString c;
    QString r;
    KAcceleratorManager::last_manage(a, c, r);

    if (automatic) { // for now we only show dialogs on F12 checks
        return;
    }

    if (c.isEmpty() && r.isEmpty() && (automatic || a.isEmpty())) {
        return;
    }

    QString s;

    if (!c.isEmpty()) {
        s += i18n("<h2>Accelerators changed</h2>")
            + QLatin1String(
                  "<table "
                  "border><tr><th><b>%1</b></th><th><b>%2</b></th></tr>%3</table>")
                  .arg(i18n("Old Text"), i18n("New Text"), c);
    }

    if (!r.isEmpty()) {
        s += i18n("<h2>Accelerators removed</h2>") + QLatin1String("<table border><tr><th><b>%1</b></th></tr>%2</table>").arg(i18n("Old Text"), r);
    }

    if (!a.isEmpty()) {
        s += i18n("<h2>Accelerators added (just for your info)</h2>")
            + QLatin1String("<table border><tr><th><b>%1</b></th></tr>%2</table>").arg(i18n("New Text"), a);
    }

    createDialog(actWin, automatic);
    drklash_view->setHtml(s);
    drklash->show();
    drklash->raise();

    // dlg will be destroyed before returning
}

void KCheckAccelerators::initiateIfNeeded()
{
    static QPointer<KCheckAccelerators> checker;
    if (checker) {
        return;
    }

    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("Development"));
    QString sKey = cg.readEntry("CheckAccelerators").trimmed();
    int key = 0;
    if (!sKey.isEmpty()) {
        QList<QKeySequence> cuts = QKeySequence::listFromString(sKey);
        if (!cuts.isEmpty()) {
            key = cuts.first()[0].toCombined();
        }
    }
    const bool autoCheck = cg.readEntry("AutoCheckAccelerators", true);
    if (key == 0 && !autoCheck) {
        return;
    }

    checker = new KCheckAccelerators(qApp, key, autoCheck);
}

#include "moc_kcheckaccelerators.cpp"
