/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2019 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KABOUT_PLUGIN_DIALOG_H
#define KABOUT_PLUGIN_DIALOG_H

#include <kxmlgui_export.h>
// Qt
#include <QDialog>
#include <memory>

class KPluginMetaData;
class KAboutPluginDialogPrivate;

/*!
 * \class KAboutPluginDialog
 * \inmodule KXMLGui
 *
 * \brief Standard "About Plugin" dialog box.
 *
 * This class provides a standard "About Plugin" dialog box.
 *
 * \since KXMLGUI 5.65
 */

class KXMLGUI_EXPORT KAboutPluginDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * Defines some options which can be applied to the about dialog
     *
     * \value NoOptions No options, show the standard about dialog
     * \value HideTranslators Don't show the translators tab
     */
    enum Option {
        NoOptions = 0x0,
        HideTranslators = 0x1,
    };
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Options)

    /*!
     * Constructor. Creates a fully featured "About Plugin" dialog box.
     *
     * \a pluginMetaData the data about the plugin to show in the dialog.
     *
     * \a options options to apply, such as hiding the translators tab.
     *
     * \a parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     */
    explicit KAboutPluginDialog(const KPluginMetaData &pluginMetaData, Options options, QWidget *parent = nullptr);

    /*!
     * Constructor. Creates a fully featured "About Plugin" dialog box.
     *
     * \a pluginMetaData the data about the plugin to show in the dialog.
     *
     * \a parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     */
    explicit KAboutPluginDialog(const KPluginMetaData &pluginMetaData, QWidget *parent = nullptr);

    ~KAboutPluginDialog() override;

private:
    std::unique_ptr<KAboutPluginDialogPrivate> const d;

    Q_DISABLE_COPY(KAboutPluginDialog)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAboutPluginDialog::Options)

#endif
