/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1997 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Sven Radej <radej@kde.org>
    SPDX-FileCopyrightText: 1997-2000 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 1999 Chris Schlaeger <cs@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KMAINWINDOW_P_H
#define KMAINWINDOW_P_H

#include <kconfiggroup.h>
#include <QPointer>
#include <QEventLoopLocker>

#define K_D(Class) Class##Private * const d = k_func()

class QObject;
class QSessionManager;
class QTimer;
class KHelpMenu;
class KMainWindow;

class KMainWindowPrivate
{
public:
    bool autoSaveSettings: 1;
    bool settingsDirty: 1;
    bool autoSaveWindowSize: 1;
    bool sizeApplied: 1;
    bool suppressCloseEvent: 1;
    KConfigGroup autoSaveGroup;
    QTimer *settingsTimer;
    QTimer *sizeTimer;
    QRect defaultWindowSize;
    KHelpMenu *helpMenu;
    KMainWindow *q;
    QPointer<QObject> dockResizeListener;
    QString dbusName;
    bool letDirtySettings;
    QEventLoopLocker locker;

    // This slot will be called when the style KCM changes settings that need
    // to be set on the already running applications.
    void _k_slotSettingsChanged(int category);
    void _k_slotSaveAutoSaveSize();
    void _k_slotSaveAutoSavePosition();

    void init(KMainWindow *_q);
    void polish(KMainWindow *q);
    enum CallCompression {
        NoCompressCalls = 0,
        CompressCalls
    };
    void setSettingsDirty(CallCompression callCompression = NoCompressCalls);
    void setSizeDirty();
};

class KMWSessionManager : public QObject
{
    Q_OBJECT
public:
    KMWSessionManager();
    ~KMWSessionManager();

private Q_SLOTS:
    void saveState(QSessionManager &);
    void commitData(QSessionManager &);
};

#endif

