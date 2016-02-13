/* This file is part of the KDE libraries
    Copyright (C) 2008 Alexander Dymo <adymo@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
#include "kshortcutschemeshelper_p.h"

#include <QAction>
#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QtXml/QDomDocument>
#include <QStandardPaths>

#include <QDir>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

#include "kactioncollection.h"
#include "kxmlguiclient.h"
#include "debug.h"

bool KShortcutSchemesHelper::saveShortcutScheme(const QList<KActionCollection *> &collections,
        const QString &schemeName)
{
    // Every action collection is associated with a KXMLGUIClient
    // (at least if it was added by KXMLGUIFactory::configureShortcuts)

    // Some GUI clients have the same name (e.g. the child client for a mainwindow
    // holding the actions for hiding/showing toolbars), so we need to save them
    // together, otherwise they will overwrite each other's file on disk.

    QMap<QString, KActionCollection *> collectionsByClientName;
    foreach (KActionCollection *coll, collections) {
        const KXMLGUIClient *client = coll->parentGUIClient();
        if (client) {
            collectionsByClientName.insertMulti(client->componentName(), coll);
        }
    }
    foreach (const QString &componentName, collectionsByClientName.uniqueKeys()) {
        const QString schemeFileName = shortcutSchemeFileName(componentName, schemeName);
        qCDebug(DEBUG_KXMLGUI) << "saving to" << schemeFileName;
        QDir().mkpath(QFileInfo(schemeFileName).absolutePath());
        QFile schemeFile(schemeFileName);
        if (!schemeFile.open(QFile::WriteOnly | QFile::Truncate)) {
            qCDebug(DEBUG_KXMLGUI) << "COULD NOT WRITE" << schemeFileName;
            return false;
        }
        QDomDocument doc;
        QDomElement docElem = doc.createElement(QStringLiteral("kpartgui"));
        docElem.setAttribute(QStringLiteral("version"), QStringLiteral("1"));
        docElem.setAttribute(QStringLiteral("name"), componentName);
        doc.appendChild(docElem);
        QDomElement elem = doc.createElement(QStringLiteral("ActionProperties"));
        docElem.appendChild(elem);

        foreach (KActionCollection *collection, collectionsByClientName.values(componentName)) {
            qCDebug(DEBUG_KXMLGUI) << "Saving shortcut scheme for action collection with" << collection->actions().count() << "actions";

            foreach (QAction *action, collection->actions()) {
                if (!action) {
                    continue;
                }

                QString actionName = action->objectName();
                QString shortcut = QKeySequence::listToString(action->shortcuts());
                qCDebug(DEBUG_KXMLGUI) << "action" << actionName << "has shortcut" << shortcut;
                if (!shortcut.isEmpty()) {
                    QDomElement act_elem = doc.createElement(QStringLiteral("Action"));
                    act_elem.setAttribute(QStringLiteral("name"), actionName);
                    act_elem.setAttribute(QStringLiteral("shortcut"), shortcut);
                    elem.appendChild(act_elem);
                }
            }
        }

        QTextStream out(&schemeFile);
        out << doc.toString(2);
    }
    return true;
}

QString KShortcutSchemesHelper::currentShortcutSchemeName()
{
    return KSharedConfig::openConfig()->group("Shortcut Schemes").readEntry("Current Scheme", "Default");
}

QString KShortcutSchemesHelper::shortcutSchemeFileName(const QString &componentName, const QString &schemeName)
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') +
           componentName + QStringLiteral("/shortcuts/") +
           schemeName;
}

QString KShortcutSchemesHelper::applicationShortcutSchemeFileName(const QString &schemeName)
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') +
           QCoreApplication::applicationName() + QStringLiteral("/shortcuts/") +
           schemeName;
}
