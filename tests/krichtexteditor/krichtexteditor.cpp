/**
    KDE Rich Text Editor
    SPDX-FileCopyrightText: 2008 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "krichtexteditor.h"
#include <krichtextwidget.h>

#include <kactioncollection.h>
#include <kstandardaction.h>

#include <QApplication>
#include <QFileDialog>
#include <QLabel>
#include <QTextStream>
#include <QTest>
#include <QSaveFile>
#include <QStatusBar>

KRichTextEditor::KRichTextEditor() : KXmlGuiWindow()
{
    setupActions();

    textArea = new KRichTextWidget(this);
    setCentralWidget(textArea);

    actionCollection()->addActions(textArea->createActions());

    setupGUI(KXmlGuiWindow::Default, QFINDTESTDATA("krichtexteditorui.rc"));

    itemLabel = new QLabel;
    statusBar()->addWidget(itemLabel);

    connect(textArea, SIGNAL(cursorPositionChanged()),
            SLOT(cursorPositionChanged()));
}

KRichTextEditor::~KRichTextEditor()
{
}

void KRichTextEditor::setupActions()
{
    KStandardAction::quit(qApp, SLOT(quit()),
                          actionCollection());

    KStandardAction::open(this, SLOT(openFile()),
                          actionCollection());

    KStandardAction::save(this, SLOT(saveFile()),
                          actionCollection());

    KStandardAction::saveAs(this, SLOT(saveFileAs()),
                            actionCollection());

    KStandardAction::openNew(this, SLOT(newFile()),
                             actionCollection());
}

void KRichTextEditor::cursorPositionChanged()
{
// Show link target in status bar
    if (textArea->textCursor().charFormat().isAnchor()) {
        QString text = textArea->currentLinkText();
        QString url = textArea->currentLinkUrl();
        itemLabel->setText(text + QStringLiteral(" -> ") + url);
    } else {
        itemLabel->setText(QString());
    }
}

void KRichTextEditor::newFile()
{
//maybeSave
    fileName.clear();
    textArea->clear();
}

void KRichTextEditor::saveFileAs(const QString &outputFileName)
{
    QSaveFile file(outputFileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    QByteArray outputByteArray;
    outputByteArray.append(textArea->toHtml().toUtf8());
    file.write(outputByteArray);
    if (!file.commit()) {
        return;
    }

    fileName = outputFileName;
}

void KRichTextEditor::saveFileAs()
{
    saveFileAs(QFileDialog::getSaveFileName());
}

void KRichTextEditor::saveFile()
{
    if (!fileName.isEmpty()) {
        saveFileAs(fileName);
    } else {
        saveFileAs();
    }
}

void KRichTextEditor::openFile()
{
    QString fileNameFromDialog = QFileDialog::getOpenFileName();
    if (fileNameFromDialog.isEmpty()) {
        return;
    }

    QFile file(fileNameFromDialog);
    if (file.open(QIODevice::ReadOnly)) {
        textArea->setTextOrHtml(QTextStream(&file).readAll());
        fileName = fileNameFromDialog;
    }
}
