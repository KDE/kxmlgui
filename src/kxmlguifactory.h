/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef kxmlguifactory_h
#define kxmlguifactory_h

#include <kxmlgui_export.h>

#include <QObject>
#include <memory>

class QAction;
class KXMLGUIFactoryPrivate;
class KXMLGUIClient;
class KXMLGUIBuilder;

class QDomAttr;
class QDomDocument;
class QDomElement;
class QDomNode;
class QDomNamedNodeMap;

namespace KXMLGUI
{
struct MergingIndex;
struct ContainerNode;
struct ContainerClient;
class BuildHelper;
}

/*!
 * \class KXMLGUIFactory
 * \inmodule KXmlGui
 *
 * KXMLGUIFactory, together with KXMLGUIClient objects, can be used to create
 * a GUI of container widgets (like menus, toolbars, etc.) and container items
 * (menu items, toolbar buttons, etc.) from an XML document and action objects.
 *
 * Each KXMLGUIClient represents a part of the GUI, composed from containers and
 * actions. KXMLGUIFactory takes care of building (with the help of a KXMLGUIBuilder)
 * and merging the GUI from an unlimited number of clients.
 *
 * Each client provides XML through a QDomDocument and actions through a
 * KActionCollection . The XML document contains the rules for how to merge the
 * GUI.
 *
 * KXMLGUIFactory processes the DOM tree provided by a client and plugs in the client's actions,
 * according to the XML and the merging rules of previously inserted clients. Container widgets
 * are built via a KXMLGUIBuilder , which has to be provided with the KXMLGUIFactory constructor.
 */
class KXMLGUI_EXPORT KXMLGUIFactory : public QObject
{
    friend class KXMLGUI::BuildHelper;
    Q_OBJECT
public:
    /*!
     * \brief Constructs a KXMLGUIFactory.
     *
     * The provided \a builder KXMLGUIBuilder with the given \a parent will be called
     * for creating and removing container widgets when clients are added/removed from the GUI.
     *
     * Note that the ownership of the given KXMLGUIBuilder object won't be transferred to this
     * KXMLGUIFactory, so you have to take care of deleting it properly.
     */
    explicit KXMLGUIFactory(KXMLGUIBuilder *builder, QObject *parent = nullptr);

    /*!
     * \brief Destructor.
     */
    ~KXMLGUIFactory() override;

    // XXX move to somewhere else? (Simon)
    /// \internal
    static QString readConfigFile(const QString &filename, const QString &componentName = QString());
    /// \internal
    static bool saveConfigFile(const QDomDocument &doc, const QString &filename, const QString &componentName = QString());

    /*!
     * \internal
     * Find or create the ActionProperties element, used when saving custom action properties
     */
    static QDomElement actionPropertiesElement(QDomDocument &doc);

    /*!
     * \internal
     * Find or create the element for a given action, by name.
     * Used when saving custom action properties
     */
    static QDomElement findActionByName(QDomElement &elem, const QString &sName, bool create);

    /*!
     * \brief Creates the GUI described by the QDomDocument of the \a client,
     * using the client's actions, and merges it with the previously
     * created GUI.
     *
     * This also means that the order in which clients are added to the factory
     * is relevant; assuming that your application supports plugins, you should
     * first add your application to the factory and then the plugin, so that the
     * plugin's UI is merged into the UI of your application, and not the other
     * way round.
     */
    void addClient(KXMLGUIClient *client);

    /*!
     * \brief Removes the GUI described by the \a client by unplugging all
     * provided actions and removing all owned containers (and storing
     * container state information in the given client).
     */
    void removeClient(KXMLGUIClient *client);

    void plugActionList(KXMLGUIClient *client, const QString &name, const QList<QAction *> &actionList);
    void unplugActionList(KXMLGUIClient *client, const QString &name);

    /*!
     * \brief Returns a list of all clients currently added to this factory.
     */
    QList<KXMLGUIClient *> clients() const;

    /*!
     * \brief Use this method to get access to a container widget with the
     * name specified with \a containerName that is owned by the \a client.
     *
     * The container name is specified with a "name" attribute in the
     * XML document.
     *
     * This function is particularly useful for getting hold of a popupmenu
     * defined in an XMLUI file.
     *
     * For instance:
     * \code
     * QMenu *popup = static_cast<QMenu*>(guiFactory()->container("my_popup",this));
     * \endcode
     * Where "my_popup" is the name of the menu in the XMLUI file, and
     * "this" is XMLGUIClient which owns the popupmenu (e.g. the mainwindow, or the part, or the plugin...).
     *
     * \a containerName The name of the container widget.
     *
     * \a client Owner of the container widget.
     *
     * \a useTagName Whether to compare the specified name with the name attribute
     * or the tag name.
     *
     * This method may return nullptr if no container with the given name
     * exists or if the container is not owned by the client.
     */
    QWidget *container(const QString &containerName, KXMLGUIClient *client, bool useTagName = false);

    QList<QWidget *> containers(const QString &tagName);

    /*!
     * \brief Use this method to free all memory allocated by the KXMLGUIFactory.
     *
     * This deletes the internal node tree and therefore resets the
     * internal state of the class. Please note that the actual GUI is
     * NOT touched at all, meaning no containers are deleted nor any
     * actions unplugged. That is something you have to do on your own.
     * So use this method only if you know what you are doing :-)
     *
     * \note This will call KXMLGUIClient::setFactory(nullptr) for all inserted clients).
     */
    void reset();

    /*!
     * \brief Free all memory allocated by the KXMLGUIFactory for a container
     * named \a containerName, including all child containers and actions.
     *
     * Additionally, you may set whether to compare the specified \a containerName
     * with the name attribute or the tag name.
     *
     * This deletes the internal node subtree for the specified container.
     * The actual GUI is not touched, no containers are deleted
     * or any actions unplugged.
     *
     * Use this method only if you know what you are doing :-)
     *
     * \a useTagName Whether to compare the specified name with the name attribute.
     *
     * \note This will call KXMLGUIClient::setFactory(nullptr) for all clients of the
     * container).
     */
    void resetContainer(const QString &containerName, bool useTagName = false);

    /*!
     * \brief Use this method to reset and reread action properties
     * (shortcuts, etc.) for all actions.
     *
     * This is needed, for example, when you change shortcuts scheme at runtime.
     */
    void refreshActionProperties();

public Q_SLOTS:
    /*!
     * \brief Shows a dialog (KShortcutsDialog) that lists every action in this factory,
     * and that can be used to change the shortcuts associated with each action.
     *
     * This slot can be connected directly to the configure shortcuts action,
     * for example:
     * \code
     * KStandardAction::keyBindings(guiFactory(), &KXMLGUIFactory::showConfigureShortcutsDialog, actionCollection());
     * \endcode
     *
     * This method constructs a KShortcutsDialog with the default arguments
     * (KShortcutsEditor::AllActions and KShortcutsEditor::LetterShortcutsAllowed).
     *
     * By default the changes will be saved back to the \c *ui.rc file
     * they were initially read from.
     *
     * If you need to run some extra code if the dialog is accepted and the settings
     * are saved, you can simply connect to the \l KXMLGUIFactory::shortcutsSaved()
     * signal before calling this method, for example:
     * \code
     * connect(guiFactory(), &KXMLGUIFactory::shortcutsSaved, this, &MyClass::slotShortcutSaved);
     * guiFactory()->showConfigureShortcutsDialog();
     * \endcode
     *
     * \sa KShortcutsDialog, KShortcutsEditor::ActionTypes, KShortcutsEditor::LetterShortcuts
     * \since 5.84
     */
    void showConfigureShortcutsDialog();

    void changeShortcutScheme(const QString &scheme);

Q_SIGNALS:
    void clientAdded(KXMLGUIClient *client);
    void clientRemoved(KXMLGUIClient *client);

    /*!
     * \brief Emitted when the factory is currently making changes to the GUI,
     * i.e. adding or removing clients.
     *
     * makingChanges(true) is emitted before any change happens, and
     * makingChanges(false) is emitted after the change is done.
     *
     * This allows e.g. KMainWindow to know that the GUI is
     * being changed programmatically and not by the user (so there is no reason to
     * save toolbar settings afterwards).
     * \since 4.1.3
     */
    void makingChanges(bool);

    /*!
     * \brief Emitted when the shortcuts have been saved (i.e. the user accepted the dialog).
     *
     * If you're using multiple instances of the same KXMLGUIClient, you probably want to
     * connect to this signal and call \c KXMLGUIClient::reloadXML() for each of your
     * KXMLGUIClients, so that the other instances update their shortcuts settings.
     *
     * \since 5.79
     */
    void shortcutsSaved();

private:
    /// Internal, called by KXMLGUIClient destructor
    KXMLGUI_NO_EXPORT void forgetClient(KXMLGUIClient *client);

private:
    friend class KXMLGUIClient;
    std::unique_ptr<KXMLGUIFactoryPrivate> const d;
};

#endif
