/* This file is part of the KDE libraries
   Copyright 2019 Friedrich W. H. Kossebau <kossebau@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KABOUT_PLUGIN_DIALOG_H
#define KABOUT_PLUGIN_DIALOG_H

#include <kxmlgui_export.h>
// Qt
#include <QDialog>

class KPluginMetaData;
class KAboutPluginDialogPrivate;

/**
 * @class KAboutPluginDialog kaboutplugindialog.h KAboutPluginDialog
 *
 * @short Standard "About Plugin" dialog box.
 *
 * This class provides a standard "About Plugin" dialog box.
 *
 * @since 5.65
 */

class KXMLGUI_EXPORT KAboutPluginDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * Defines some options which can be applied to the about dialog
     * @see Options
     */
    enum Option {
        NoOptions       = 0x0, ///< No options, show the standard about dialog
        HideTranslators = 0x1, ///< Don't show the translators tab
    };
    /**
     * Stores a combination of #Option values.
     */
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Options)

    /**
     * Constructor. Creates a fully featured "About Plugin" dialog box.
     *
     * @param pluginMetaData the data about the plugin to show in the dialog.
     * @param options options to apply, such as hiding the translators tab.
     * @param parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     */
    explicit KAboutPluginDialog(const KPluginMetaData &pluginMetaData, Options options, QWidget *parent = nullptr);

    /**
     * Constructor. Creates a fully featured "About Plugin" dialog box.
     *
     * @param pluginMetaData the data about the plugin to show in the dialog.
     * @param parent The parent of the dialog box. You should use the
     *        toplevel window so that the dialog becomes centered.
     */
    explicit KAboutPluginDialog(const KPluginMetaData &pluginMetaData, QWidget *parent = nullptr);

    ~KAboutPluginDialog() override;

private:
    const QScopedPointer<KAboutPluginDialogPrivate> d;

    Q_DISABLE_COPY(KAboutPluginDialog)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAboutPluginDialog::Options)

#endif
