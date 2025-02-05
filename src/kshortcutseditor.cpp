/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1998 Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 1997 Nicolas Hadacek <hadacek@kde.org>
    SPDX-FileCopyrightText: 1998 Matthias Ettrich <ettrich@kde.org>
    SPDX-FileCopyrightText: 2001 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2006 Hamish Rodda <rodda@kde.org>
    SPDX-FileCopyrightText: 2007 Roberto Raggi <roberto@kdevelop.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-xmlgui.h"

#include "kshortcutseditor.h"

// The following is needed for KShortcutsEditorPrivate
#include "debug.h"
#include "kshortcutsdialog_p.h"

#include <QAction>
#include <QHeaderView>
#include <QList>
#include <QObject>
#include <QPrintDialog>
#include <QPrinter>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTable>
#include <QTextTableFormat>
#include <QTimer>

#include <KConfig>
#include <KConfigGroup>
#if HAVE_GLOBALACCEL
#include <KGlobalAccel>
#endif
#include "kactioncategory.h"
#include "kactioncollection.h"
#include <KTreeWidgetSearchLine>

//---------------------------------------------------------------------
// KShortcutsEditor
//---------------------------------------------------------------------

KShortcutsEditor::KShortcutsEditor(KActionCollection *collection, QWidget *parent, ActionTypes actionType, LetterShortcuts allowLetterShortcuts)
    : QWidget(parent)
    , d(new KShortcutsEditorPrivate(this))
{
    d->initGUI(actionType, allowLetterShortcuts);
    addCollection(collection);
}

KShortcutsEditor::KShortcutsEditor(QWidget *parent, ActionTypes actionType, LetterShortcuts allowLetterShortcuts)
    : QWidget(parent)
    , d(new KShortcutsEditorPrivate(this))
{
    d->initGUI(actionType, allowLetterShortcuts);
}

KShortcutsEditor::~KShortcutsEditor()
{
    // reset all pending changes
    undo();
}

bool KShortcutsEditor::isModified() const
{
    // Iterate over all items
    QTreeWidgetItemIterator it(d->ui.list, QTreeWidgetItemIterator::NoChildren);

    for (; (*it); ++it) {
        KShortcutsEditorItem *item = dynamic_cast<KShortcutsEditorItem *>(*it);
        if (item && item->isModified()) {
            return true;
        }
    }
    return false;
}

void KShortcutsEditor::clearCollections()
{
    d->delegate->contractAll();
    d->ui.list->clear();
    d->actionCollections.clear();
    QTimer::singleShot(0, this, &KShortcutsEditor::resizeColumns);
}

void KShortcutsEditor::addCollection(KActionCollection *collection, const QString &title)
{
    // KXmlGui add action collections unconditionally. If some plugin doesn't
    // provide actions we don't want to create empty subgroups.
    if (collection->isEmpty()) {
        return;
    }

    // We add a bunch of items. Prevent the treewidget from permanently
    // updating.
    setUpdatesEnabled(false);

    d->actionCollections.append(collection);
    // Forward our actionCollections to the delegate which does the conflict
    // checking.
    d->delegate->setCheckActionCollections(d->actionCollections);
    QString displayTitle = title;
    if (displayTitle.isEmpty()) {
        // Use the programName (Translated).
        displayTitle = collection->componentDisplayName();
    }

    KShortcutsEditorPrivate::HierarchyInfo hierarchy;
    hierarchy.root = d->ui.list->invisibleRootItem();
    hierarchy.program = d->findOrMakeItem(hierarchy.root, displayTitle);
    hierarchy.action = nullptr;

    // Set to remember which actions we have seen.
    QSet<QAction *> actionsSeen;

    // Add all categories in their own subtree below the collections root node
    const QList<KActionCategory *> categories = collection->findChildren<KActionCategory *>();
    for (KActionCategory *category : categories) {
        hierarchy.action = d->findOrMakeItem(hierarchy.program, category->text());
        const auto categoryActions = category->actions();
        for (QAction *action : categoryActions) {
            // Set a marker that we have seen this action
            actionsSeen.insert(action);
            d->addAction(action, hierarchy.action);
        }
    }

    // The rest of the shortcuts are added as direct children of the action
    // collections root node
    const auto collectionActions = collection->actions();
    for (QAction *action : collectionActions) {
        if (actionsSeen.contains(action)) {
            continue;
        }

        d->addAction(action, hierarchy.program);
    }

    // sort the list
    d->ui.list->sortItems(Name, Qt::AscendingOrder);

    // Hide Global shortcuts columns if there are no global shortcuts
    d->setGlobalColumnsHidden(!d->m_hasAnyGlobalShortcuts);

    // ...And hide local shortcuts columns if there are no local shortcuts
    d->setLocalColumnsHidden(!d->m_hasAnyLocalShortcuts);

    // re-enable updating
    setUpdatesEnabled(true);

    QTimer::singleShot(0, this, &KShortcutsEditor::resizeColumns);
}

void KShortcutsEditor::importConfiguration(KConfigBase *config)
{
    d->importConfiguration(config);
}

void KShortcutsEditor::exportConfiguration(KConfigBase *config) const
{
    Q_ASSERT(config);
    if (!config) {
        return;
    }

    if (d->actionTypes & KShortcutsEditor::GlobalAction) {
        QString groupName(QStringLiteral("Global Shortcuts"));
        KConfigGroup group(config, groupName);
        for (KActionCollection *collection : std::as_const(d->actionCollections)) {
            collection->exportGlobalShortcuts(&group, true);
        }
    }
    if (d->actionTypes & ~KShortcutsEditor::GlobalAction) {
        QString groupName(QStringLiteral("Shortcuts"));
        KConfigGroup group(config, groupName);
        for (KActionCollection *collection : std::as_const(d->actionCollections)) {
            collection->writeSettings(&group, true);
        }
    }
}

void KShortcutsEditor::writeConfiguration(KConfigGroup *config) const
{
    for (KActionCollection *collection : std::as_const(d->actionCollections)) {
        collection->writeSettings(config);
    }
}

// slot
void KShortcutsEditor::resizeColumns()
{
    // skip Name column as its section resize mode will take care of resizing to contents
    for (int i = Name + 1; i < d->ui.list->columnCount(); i++) {
        if (d->ui.list->isColumnHidden(i)) {
            continue;
        }
        d->ui.list->resizeColumnToContents(i);
    }
}

void KShortcutsEditor::save()
{
    writeConfiguration();
    // we have to call commit. If we wouldn't do that the changes would be
    // undone on deletion! That would lead to weird problems. Changes to
    // Global Shortcuts would vanish completely. Changes to local shortcuts
    // would vanish for this session.
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if (KShortcutsEditorItem *item = dynamic_cast<KShortcutsEditorItem *>(*it)) {
            item->commit();
        }
    }
}

void KShortcutsEditor::undo()
{
    // This function used to crash sometimes when invoked by clicking on "cancel"
    // with Qt 4.2.something. Apparently items were deleted too early by Qt.
    // It seems to work with 4.3-ish Qt versions. Keep an eye on this.
    for (QTreeWidgetItemIterator it(d->ui.list); (*it); ++it) {
        if (KShortcutsEditorItem *item = dynamic_cast<KShortcutsEditorItem *>(*it)) {
            item->undo();
        }
    }
}

// We ask the user here if there are any conflicts, as opposed to undo().
// They don't do the same thing anyway, this just not to confuse any readers.
// slot
void KShortcutsEditor::allDefault()
{
    d->allDefault();
}

void KShortcutsEditor::printShortcuts() const
{
    d->printShortcuts();
}

KShortcutsEditor::ActionTypes KShortcutsEditor::actionTypes() const
{
    return d->actionTypes;
}

void KShortcutsEditor::setActionTypes(ActionTypes actionTypes)
{
    d->setActionTypes(actionTypes);
}

//---------------------------------------------------------------------
// KShortcutsEditorPrivate
//---------------------------------------------------------------------

KShortcutsEditorPrivate::KShortcutsEditorPrivate(KShortcutsEditor *qq)
    : q(qq)
    , delegate(nullptr)
{
}

void KShortcutsEditorPrivate::initGUI(KShortcutsEditor::ActionTypes types, KShortcutsEditor::LetterShortcuts allowLetterShortcuts)
{
    actionTypes = types;

    ui.setupUi(q);
    q->layout()->setContentsMargins(0, 0, 0, 0);
    ui.searchFilter->searchLine()->setTreeWidget(ui.list); // Plug into search line
    ui.list->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui.list->header()->hideSection(ShapeGesture); // mouse gestures didn't make it in time...
    ui.list->header()->hideSection(RockerGesture);

#if HAVE_GLOBALACCEL
    const bool hideGlobals = !(actionTypes & KShortcutsEditor::GlobalAction);
#else
    const bool hideGlobals = true;
#endif

    if (hideGlobals) {
        setGlobalColumnsHidden(true);
    } else if (!(actionTypes & ~KShortcutsEditor::GlobalAction)) {
        setLocalColumnsHidden(true);
    }

    // Create the Delegate. It is responsible for the KKeySeqeunceWidgets that
    // really change the shortcuts.
    delegate = new KShortcutsEditorDelegate(ui.list, allowLetterShortcuts == KShortcutsEditor::LetterShortcutsAllowed);

    ui.list->setItemDelegate(delegate);
    ui.list->setSelectionBehavior(QAbstractItemView::SelectItems);
    ui.list->setSelectionMode(QAbstractItemView::SingleSelection);
    // we have our own editing mechanism
    ui.list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.list->setAlternatingRowColors(true);

    // TODO listen to changes to global shortcuts
    QObject::connect(delegate, &KShortcutsEditorDelegate::shortcutChanged, q, [this](const QVariant &newShortcut, const QModelIndex &index) {
        capturedShortcut(newShortcut, index);
    });
    // hide the editor widget chen its item becomes hidden
    QObject::connect(ui.searchFilter->searchLine(), &KTreeWidgetSearchLine::hiddenChanged, delegate, &KShortcutsEditorDelegate::hiddenBySearchLine);

    ui.searchFilter->setFocus();
}

void KShortcutsEditorPrivate::setGlobalColumnsHidden(bool hide)
{
    QHeaderView *header = ui.list->header();
    header->setSectionHidden(GlobalPrimary, hide);
    header->setSectionHidden(GlobalAlternate, hide);
}

void KShortcutsEditorPrivate::setLocalColumnsHidden(bool hide)
{
    QHeaderView *header = ui.list->header();
    header->setSectionHidden(LocalPrimary, hide);
    header->setSectionHidden(LocalAlternate, hide);
}

void KShortcutsEditorPrivate::setActionTypes(KShortcutsEditor::ActionTypes types)
{
    if (actionTypes == types) {
        return;
    }
    actionTypes = types;

    // Show/hide columns based on the newly set action types
    setGlobalColumnsHidden(!(actionTypes & KShortcutsEditor::GlobalAction));
    setLocalColumnsHidden(!(actionTypes & ~KShortcutsEditor::GlobalAction));
}

bool KShortcutsEditorPrivate::addAction(QAction *action, QTreeWidgetItem *parent)
{
    // If the action name starts with unnamed- spit out a warning and ignore
    // it. That name will change at will and will break loading and writing
    QString actionName = action->objectName();
    if (actionName.isEmpty() || actionName.startsWith(QLatin1String("unnamed-"))) {
        qCCritical(DEBUG_KXMLGUI) << "Skipping action without name " << action->text() << "," << actionName << "!";
        return false;
    }

    const QVariant value = action->property("isShortcutConfigurable");
    if (!value.isValid() || value.toBool()) {
        new KShortcutsEditorItem(parent, action);

#if HAVE_GLOBALACCEL
        if (!m_hasAnyGlobalShortcuts) { // If one global action was found, skip
            m_hasAnyGlobalShortcuts = KGlobalAccel::self()->hasShortcut(action);
        }

        if (!m_hasAnyLocalShortcuts) { // If one local action was found, skip
            m_hasAnyLocalShortcuts = !KGlobalAccel::self()->hasShortcut(action);
        }
#else
        m_hasAnyLocalShortcuts = true;
#endif

        return true;
    }

    return false;
}

void KShortcutsEditorPrivate::allDefault()
{
    for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
        if (!(*it)->parent() || (*it)->type() != ActionItem) {
            continue;
        }

        KShortcutsEditorItem *item = static_cast<KShortcutsEditorItem *>(*it);
        QAction *act = item->m_action;

        QList<QKeySequence> defaultShortcuts = act->property("defaultShortcuts").value<QList<QKeySequence>>();
        if (act->shortcuts() != defaultShortcuts) {
            QKeySequence primary = defaultShortcuts.isEmpty() ? QKeySequence() : defaultShortcuts.at(0);
            QKeySequence alternate = defaultShortcuts.size() <= 1 ? QKeySequence() : defaultShortcuts.at(1);
            changeKeyShortcut(item, LocalPrimary, primary);
            changeKeyShortcut(item, LocalAlternate, alternate);
        }

#if HAVE_GLOBALACCEL
        if (KGlobalAccel::self()->shortcut(act) != KGlobalAccel::self()->defaultShortcut(act)) {
            QList<QKeySequence> defaultShortcut = KGlobalAccel::self()->defaultShortcut(act);
            changeKeyShortcut(item, GlobalPrimary, primarySequence(defaultShortcut));
            changeKeyShortcut(item, GlobalAlternate, alternateSequence(defaultShortcut));
        }
#endif
    }
}

// static
KShortcutsEditorItem *KShortcutsEditorPrivate::itemFromIndex(QTreeWidget *const w, const QModelIndex &index)
{
    QTreeWidgetItem *item = w->itemFromIndex(index);
    if (item && item->type() == ActionItem) {
        return static_cast<KShortcutsEditorItem *>(item);
    }
    return nullptr;
}

QTreeWidgetItem *KShortcutsEditorPrivate::findOrMakeItem(QTreeWidgetItem *parent, const QString &name)
{
    for (int i = 0; i < parent->childCount(); i++) {
        QTreeWidgetItem *child = parent->child(i);
        if (child->text(0) == name) {
            return child;
        }
    }
    QTreeWidgetItem *ret = new QTreeWidgetItem(parent, NonActionItem);
    ret->setText(0, name);
    ui.list->expandItem(ret);
    ret->setFlags(ret->flags() & ~Qt::ItemIsSelectable);
    return ret;
}

// private slot
void KShortcutsEditorPrivate::capturedShortcut(const QVariant &newShortcut, const QModelIndex &index)
{
    // dispatch to the right handler
    if (!index.isValid()) {
        return;
    }
    int column = index.column();
    KShortcutsEditorItem *item = itemFromIndex(ui.list, index);
    Q_ASSERT(item);

    if (column >= LocalPrimary && column <= GlobalAlternate) {
        changeKeyShortcut(item, column, newShortcut.value<QKeySequence>());
    }
}

void KShortcutsEditorPrivate::changeKeyShortcut(KShortcutsEditorItem *item, uint column, const QKeySequence &capture)
{
    // The keySequence we get is cleared by KKeySequenceWidget. No conflicts.
    if (capture == item->keySequence(column)) {
        return;
    }

    item->setKeySequence(column, capture);
    Q_EMIT q->keyChange();
    // force view update
    item->setText(column, capture.toString(QKeySequence::NativeText));
}

void KShortcutsEditorPrivate::importConfiguration(KConfigBase *config)
{
    Q_ASSERT(config);
    if (!config) {
        return;
    }

    KConfigGroup globalShortcutsGroup(config, QStringLiteral("Global Shortcuts"));
    if ((actionTypes & KShortcutsEditor::GlobalAction) && globalShortcutsGroup.exists()) {
        for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
            if (!(*it)->parent()) {
                continue;
            }

            KShortcutsEditorItem *item = static_cast<KShortcutsEditorItem *>(*it);
            const QString actionId = item->data(Id).toString();
            if (!globalShortcutsGroup.hasKey(actionId)) {
                continue;
            }

            QList<QKeySequence> sc = QKeySequence::listFromString(globalShortcutsGroup.readEntry(actionId, QString()));
            changeKeyShortcut(item, GlobalPrimary, primarySequence(sc));
            changeKeyShortcut(item, GlobalAlternate, alternateSequence(sc));
        }
    }

    if (actionTypes & ~KShortcutsEditor::GlobalAction) {
        const KConfigGroup localShortcutsGroup(config, QStringLiteral("Shortcuts"));
        for (QTreeWidgetItemIterator it(ui.list); (*it); ++it) {
            if (!(*it)->parent()) {
                continue;
            }
            KShortcutsEditorItem *item = static_cast<KShortcutsEditorItem *>(*it);
            const QString actionId = item->data(Id).toString();
            if (!localShortcutsGroup.hasKey(actionId)) {
                continue;
            }

            QList<QKeySequence> sc = QKeySequence::listFromString(localShortcutsGroup.readEntry(actionId, QString()));
            changeKeyShortcut(item, LocalPrimary, primarySequence(sc));
            changeKeyShortcut(item, LocalAlternate, alternateSequence(sc));
        }
    }
}

/*TODO for the printShortcuts function
Nice to have features (which I'm not sure I can do before may due to
more important things):

- adjust the general page borders, IMHO they're too wide

- add a custom printer options page that allows to filter out all
  actions that don't have a shortcut set to reduce this list. IMHO this
  should be optional as people might want to simply print all and  when
  they find a new action that they assign a shortcut they can simply use
  a pen to fill out the empty space

- find a way to align the Main/Alternate/Global entries in the shortcuts
  column without adding borders. I first did this without a nested table
  but instead simply added 3 rows and merged the 3 cells in the Action
  name and description column, but unfortunately I didn't find a way to
  remove the borders between the 6 shortcut cells.
*/
void KShortcutsEditorPrivate::printShortcuts() const
{
// One can't print on wince
#ifndef _WIN32_WCE
    QTreeWidgetItem *root = ui.list->invisibleRootItem();
    QTextDocument doc;

    doc.setDefaultFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont));

    QTextCursor cursor(&doc);
    cursor.beginEditBlock();
    QTextCharFormat headerFormat;
    headerFormat.setProperty(QTextFormat::FontSizeAdjustment, 3);
    headerFormat.setFontWeight(QFont::Bold);
    cursor.insertText(i18nc("header for an applications shortcut list", "Shortcuts for %1", QGuiApplication::applicationDisplayName()), headerFormat);
    QTextCharFormat componentFormat;
    componentFormat.setProperty(QTextFormat::FontSizeAdjustment, 2);
    componentFormat.setFontWeight(QFont::Bold);
    QTextBlockFormat componentBlockFormat = cursor.blockFormat();
    componentBlockFormat.setTopMargin(16);
    componentBlockFormat.setBottomMargin(16);

    QTextTableFormat tableformat;
    tableformat.setHeaderRowCount(1);
    tableformat.setCellPadding(4.0);
    tableformat.setCellSpacing(0);
    tableformat.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
    tableformat.setBorder(0.5);

    struct ColumnInfo {
        QString title;
        ColumnDesignation column;
    };
    const ColumnInfo shortcutTitleToColumnMap[] = {
        {i18n("Main:"), LocalPrimary},
        {i18n("Alternate:"), LocalAlternate},
        {i18n("Global:"), GlobalPrimary},
        {i18n("Global alternate:"), GlobalAlternate},
    };

    for (int i = 0; i < root->childCount(); i++) {
        QTreeWidgetItem *item = root->child(i);
        cursor.insertBlock(componentBlockFormat, componentFormat);
        cursor.insertText(item->text(0));

        QTextTable *table = cursor.insertTable(1, 3);
        table->setFormat(tableformat);
        int currow = 0;

        QTextTableCell cell = table->cellAt(currow, 0);
        QTextCharFormat format = cell.format();
        format.setFontWeight(QFont::Bold);
        cell.setFormat(format);
        cell.firstCursorPosition().insertText(i18n("Action Name"));

        cell = table->cellAt(currow, 1);
        cell.setFormat(format);
        cell.firstCursorPosition().insertText(i18n("Shortcuts"));

        cell = table->cellAt(currow, 2);
        cell.setFormat(format);
        cell.firstCursorPosition().insertText(i18n("Description"));
        currow++;

        for (QTreeWidgetItemIterator it(item); *it; ++it) {
            if ((*it)->type() != ActionItem) {
                continue;
            }

            KShortcutsEditorItem *editoritem = static_cast<KShortcutsEditorItem *>(*it);
            table->insertRows(table->rows(), 1);
            QVariant data = editoritem->data(Name, Qt::DisplayRole);
            table->cellAt(currow, 0).firstCursorPosition().insertText(data.toString());

            QTextTable *shortcutTable = nullptr;
            for (const auto &[title, column] : shortcutTitleToColumnMap) {
                data = editoritem->data(column, Qt::DisplayRole);
                QString key = data.value<QKeySequence>().toString();

                if (!key.isEmpty()) {
                    if (!shortcutTable) {
                        shortcutTable = table->cellAt(currow, 1).firstCursorPosition().insertTable(1, 2);
                        QTextTableFormat shortcutTableFormat = tableformat;
                        shortcutTableFormat.setCellSpacing(0.0);
                        shortcutTableFormat.setHeaderRowCount(0);
                        shortcutTableFormat.setBorder(0.0);
                        shortcutTable->setFormat(shortcutTableFormat);
                    } else {
                        shortcutTable->insertRows(shortcutTable->rows(), 1);
                    }
                    shortcutTable->cellAt(shortcutTable->rows() - 1, 0).firstCursorPosition().insertText(title);
                    shortcutTable->cellAt(shortcutTable->rows() - 1, 1).firstCursorPosition().insertText(key);
                }
            }

            QAction *action = editoritem->m_action;
            cell = table->cellAt(currow, 2);
            format = cell.format();
            format.setProperty(QTextFormat::FontSizeAdjustment, -1);
            cell.setFormat(format);
            cell.firstCursorPosition().insertHtml(action->whatsThis());

            currow++;
        }
        cursor.movePosition(QTextCursor::End);
    }
    cursor.endEditBlock();

    QPrinter printer;
    QPrintDialog *dlg = new QPrintDialog(&printer, q);
    if (dlg->exec() == QDialog::Accepted) {
        doc.print(&printer);
    }
    delete dlg;
#endif
}

#include "moc_kshortcutseditor.cpp"
