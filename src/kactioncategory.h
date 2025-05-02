/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2008 Michael Jansen <kde@michael-jansen.biz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KACTIONCATEGORY_H
#define KACTIONCATEGORY_H

#include <kxmlgui_export.h>

#include <QList>
#include <QObject>
#include <QString>
#include <memory>

#include <KStandardAction>

#include "kactioncollection.h"

struct KActionCategoryPrivate;

class QAction;

/*!
 * \class KActionCategory
 *
 * \brief Categorize actions for KShortcutsEditor.
 *
 * KActionCategory provides a second level to organize the actions in
 * KShortcutsEditor.
 *
 * The first possibility is using more than one action collection. Each
 * actions collection becomes a top level node.
 *
 * TODO qdoc
 * + action collection 1
 *   + first action
 *   + second action
 *   + third action
 * + action collection 2
 *   + first action
 *   + second action
 *   + third action
 *
 * Using KActionCategory it's possible to group the actions of one collection.
 * + action collection 1
 *   + first action
 *   + first category
 *     + action 1 in category
 *     + action 2 in category
 *   + second action
 *
 * The usage is analog to action collections. Just create a category and use
 * it instead of the collection to create the actions.
 *
 * The synchronization between KActionCollection and KActionCategory is done
 * internally. There is for example no need to remove actions from a category.
 * It is done implicitly if the action is removed from the associated
 * collection.
 *
 * \code
 *
 * KActionCategory *file = new KActionCategory(i18n("File"), actionCollection());
 * file->addAction(
 *      KStandardActions::New,   //< see KStandardAction
 *      this,                   //< Receiver
 *      &MyApp::fileNew);       //< Slot
 *
 * ... more actions added to file ...
 *
 * KActionCategory *edit = new KActionCategory(i18n("Edit"), actionCollection());
 * edit->addAction(
 *      KStandardActions::Copy,  //< see KStandardAction
 *      this,                   //< Receiver
 *      &MyApp::fileNew);       //< Slot
 *
 * ...
 *
 * \endcode
 */
class KXMLGUI_EXPORT KActionCategory : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString text READ text WRITE setText)

public:
    /*!
     * Default constructor
     */
    explicit KActionCategory(const QString &text, KActionCollection *parent = nullptr);

    ~KActionCategory() override;

    /*!
     * Add a action to the category.
     *
     * This methods are provided for your convenience. They call the
     * corresponding method of KActionCollection.
     */
    QAction *addAction(const QString &name, QAction *action);

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(6, 9)
    KXMLGUI_DEPRECATED_VERSION(6, 9, "Use KStandardActions overload")
    QAction *addAction(KStandardAction::StandardAction actionType, const QObject *receiver = nullptr, const char *member = nullptr);
#endif

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(6, 9)
    KXMLGUI_DEPRECATED_VERSION(6, 9, "Use KStandardActions overload")
    QAction *addAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver = nullptr, const char *member = nullptr);
#endif

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(6, 9)
    // not marked with KXMLGUI_DEPRECATED_VERSION, otherwise addAction("bla") generates a warning, but we don't want that
    /*!
     * @deprecated since 6.9, use PMF overload
     */
    QAction *addAction(const QString &name, const QObject *receiver = nullptr, const char *member = nullptr);
#else
    // this is only enabled when
    // addAction(KStandardAction::StandardAction actionType, const QObject *receiver = nullptr, const char *member = nullptr
    // is not built, otherwise there is ambiguity
    inline QAction *addAction(const QString &name)
    {
        QAction *action = collection()->addAction(name);
        addAction(action);
        return action;
    }
#endif

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(6, 9)
    template<class ActionType>
    KXMLGUI_DEPRECATED_VERSION(6, 9, "Use PMF-based overload")
    ActionType *add(const QString &name, const QObject *receiver, const char *member)
    {
        ActionType *action = collection()->add<ActionType>(name, receiver, member);
        addAction(action);
        return action;
    }
#endif

    /**
     * Creates a new standard action, adds it to the collection and connects the
     * action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is also returned.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * @param actionType The standard action type of the action to create.
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     * @param slot The slot or lambda to connect the triggered(bool) signal to.
     * @return the created action.
     *
     * @since 6.9
     */
#ifdef K_DOXYGEN
    inline QAction *addAction(KStandardActions::StandardAction actionType, const Receiver *receiver, Func slot)
#else
    template<class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
    addAction(KStandardActions::StandardAction actionType, const Receiver *receiver, Func slot)
#endif
    {
        QAction *action = collection()->addAction(actionType, receiver, slot);
        addAction(action);
        return action;
    }

    /**
     * Creates a new standard action and adds it to the collection.
     * The newly created action is also returned.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * @param actionType The standard action type of the action to create.
     * @return the created action.
     *
     * @since 6.9
     */
    QAction *addAction(KStandardActions::StandardAction actionType);

    /**
     * Creates a new standard action, adds it to the collection and connects the
     * action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is also returned.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * @param actionType The standard action type of the action to create.
     * @param name The name by which the action be retrieved again from the collection.
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     * @param slot The slot or lambda to connect the triggered(bool) signal to.
     * @return the created action.
     *
     * @since 6.9
     */
#ifdef K_DOXYGEN
    inline QAction *addAction(KStandardActions::StandardAction actionType, const Receiver *receiver, Func slot)
#else
    template<class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
    addAction(KStandardActions::StandardAction actionType, const QString &name, const Receiver *receiver, Func slot)
#endif
    {
        QAction *action = collection()->addAction(actionType, name, receiver, slot);
        addAction(action);
        return action;
    }

    /**
     * Creates a new action, adds it to the collection and connects the
     * action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is also returned.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * @param name The name by which the action be retrieved again from the collection.
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     * @param slot The slot or lambda to connect the triggered(bool) signal to.
     * @return the created action.
     *
     * @since 6.9
     */
#ifdef K_DOXYGEN
    inline QAction *addAction(const Receiver *receiver, Func slot)
#else
    template<class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
    addAction(const QString &name, const Receiver *receiver, Func slot)
#endif
    {
        QAction *action = collection()->addAction(name, receiver, slot);
        addAction(action);
        return action;
    }

    /**
     * Creates a new action, adds it to the collection and connects the
     * action's triggered(bool) signal to the specified receiver/member. The
     * newly created action is also returned.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * @param name The name by which the action be retrieved again from the collection.
     * @param receiver The QObject to connect the triggered(bool) signal to.  Leave nullptr if no
     *                 connection is desired.
     * @param slot The slot or lambda to connect the triggered(bool) signal to.
     * @return the created action.
     *
     * @since 6.9
     */
#ifdef K_DOXYGEN
    inline QAction *add(const Receiver *receiver, Func slot)
#else
    template<class ActionType, class Receiver, class Func>
    inline typename std::enable_if<!std::is_convertible<Func, const char *>::value, QAction>::type *
    add(const QString &name, const Receiver *receiver, Func slot)
#endif
    {
        QAction *action = collection()->add<ActionType>(name, receiver, slot);
        addAction(action);
        return action;
    }

    /**
     * Creates a new action and adds it to the collection.
     * The newly created action is also returned.
     *
     * The KActionCollection takes ownership of the action object.
     *
     * @param name The name by which the action be retrieved again from the collection.
     * @return the created action.
     *
     * @since 6.9
     */
    template<class ActionType>
    ActionType *add(const QString &name)
    {
        ActionType *action = collection()->add<ActionType>(name);
        addAction(action);
        return action;
    }

    /*!
     * Returns the actions belonging to this category
     */
    const QList<QAction *> actions() const;

    /*!
     * The action collection this category is associated with.
     */
    KActionCollection *collection() const;

    /*!
     * The action categorys descriptive text
     */
    QString text() const;

    /*!
     * Set the action categorys descriptive text.
     */
    void setText(const QString &text);

private:
    /*!
     * Remove \a action from this category if found.
     */
    KXMLGUI_NO_EXPORT void unlistAction(QAction *action);

    /*!
     * Add \a action to category
     */
    void addAction(QAction *action); // exported because called from template method ActionType *add<ActionType>(...)

private:
    //! KActionCollection needs access to some of our helper methods
    friend class KActionCollectionPrivate;

    //! Implementation details
    std::unique_ptr<KActionCategoryPrivate> const d;
};

#endif /* #ifndef KACTIONCATEGORY_H */
