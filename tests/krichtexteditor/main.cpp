/*
    KDE Rich Text Editor
    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "krichtexteditor.h"
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication::setApplicationName(QStringLiteral("krichtexteditor"));
    QApplication app(argc, argv);
    KRichTextEditor *mw = new KRichTextEditor();
    mw->show();
    return app.exec();
}
