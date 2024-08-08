/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer at kde.org>

    Parts of this class have been take from the KAboutApplication class, which was:
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KABOUT_APPLICATION_DIALOG_H
#define KABOUT_APPLICATION_DIALOG_H

#include <QDialog>
#include <memory>

#include <kxmlgui_export.h>

class KAboutData;

/*!
 * \class KAboutApplicationDialog
 *
 * \brief Standard "About Application" dialog box.
 *
 * This class provides the standard "About Application" dialog box
 * that is used by KHelpMenu. It uses the information of the global
 * KAboutData that is specified at the start of your program in
 * main(). Normally you should not use this class directly but rather
 * the KHelpMenu class or even better just subclass your toplevel
 * window from KMainWindow. If you do the latter, the help menu and
 * thereby this dialog box is available through the
 * KMainWindow::helpMenu() function.
 *
 * \image html kaboutapplicationdialog.png "KAboutApplicationDialog" TODO qdoc
 *
 */

class KXMLGUI_EXPORT KAboutApplicationDialog : public QDialog
{
    Q_OBJECT
public:
    /*!
     * Defines some options which can be applied to the about dialog
     *
     * \value NoOptions No options, show the standard about dialog
     * \value HideTranslators Don't show the translators tab
     * \value [since KXMLGUI 5.77] HideLibraries Don't show the components tab
     *
     * \since KXMLGUI 4.4
     */
    enum Option {
        NoOptions = 0x0,
        HideTranslators = 0x1,
        HideLibraries = 0x2,
    };
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Options)

    /*!
     * Constructor. Creates a fully featured "About Application" dialog box.
     *
     * \a aboutData A KAboutData object which data
     *        will be used for filling the dialog.
     *
     * \a opts Additional options that can be applied, such as hiding the KDE version
     *             or the translators tab.
     *
     * \a parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     *
     * \since KXMLGUI 4.4
     */
    explicit KAboutApplicationDialog(const KAboutData &aboutData, Options opts, QWidget *parent = nullptr);

    /*!
     * Constructor. Creates a fully featured "About Application" dialog box.
     *
     * \a aboutData A KAboutData object which data
     *        will be used for filling the dialog.
     *
     * \a parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     */
    explicit KAboutApplicationDialog(const KAboutData &aboutData, QWidget *parent = nullptr);

    ~KAboutApplicationDialog() override;

private:
    std::unique_ptr<class KAboutApplicationDialogPrivate> const d;

    Q_DISABLE_COPY(KAboutApplicationDialog)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAboutApplicationDialog::Options)

#endif
