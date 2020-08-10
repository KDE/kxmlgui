/*
    SPDX-FileCopyrightText: 2001 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kxmlguitest.h"

#include <QAction>
#include <QApplication>
#include <QLineEdit>
#include <QDir>
#include <QTest>

#include <kmainwindow.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>
#include <kxmlguibuilder.h>
#include <kactioncollection.h>

void Client::slotSec()
{
    qDebug() << "Client::slotSec()";
}

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("test"));
    QApplication app(argc, argv);
    QAction *a;

    KMainWindow *mainwindow = new KMainWindow;

    QLineEdit *line = new QLineEdit(mainwindow);
    mainwindow->setCentralWidget(line);

    mainwindow->show();

    KXMLGUIBuilder *builder = new KXMLGUIBuilder(mainwindow);

    KXMLGUIFactory *factory = new KXMLGUIFactory(builder);

    Client *shell = new Client;
    shell->setComponentName(QStringLiteral("konqueror"), QStringLiteral("Konqueror"));

    a = new QAction(QIcon::fromTheme(QStringLiteral("view-split-left-right")), QStringLiteral("Split"), shell);
    shell->actionCollection()->addAction(QStringLiteral("splitviewh"), a);

    shell->setXMLFile(QFINDTESTDATA("kxmlguitest_shell.rc"));

    factory->addClient(shell);

    Client *part = new Client;

    a = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), QStringLiteral("decfont"), part);
    part->actionCollection()->addAction(QStringLiteral("decFontSizes"), a);
    a = new QAction(QIcon::fromTheme(QStringLiteral("security-low")), QStringLiteral("sec"), part);
    part->actionCollection()->addAction(QStringLiteral("security"), a);
    part->actionCollection()->setDefaultShortcuts(a, QList<QKeySequence>() << Qt::ALT + Qt::Key_1);
    a->connect(a, SIGNAL(triggered(bool)), part, SLOT(slotSec()));

    part->setXMLFile(QFINDTESTDATA("kxmlguitest_part.rc"));

    factory->addClient(part);
    for (int i = 0; i < 10; ++i) {
        factory->removeClient(part);
        factory->addClient(part);
    }

    return app.exec();
}
