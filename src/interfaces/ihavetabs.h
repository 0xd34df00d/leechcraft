/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_IHAVETABS_H
#define INTERFACES_IHAVETABS_H
#include <QMetaType>
#include <QList>
#include <QMap>
#include <QByteArray>
#include <QIcon>

namespace LC
{
	/** @brief Defines different behavior features of tab classes.
	 */
	enum TabFeature
	{
		/** @brief No special features.
		 */
		TFEmpty = 0x0,

		/** @brief This tab could be opened by user request.
		 *
		 * If tab class has this feature, a corresponding action in new
		 * tab menu would be created to allow the user to open this tab.
		 *
		 * If tab class doesn't have this feature, the only way for the
		 * tab to be opened is for the corresponding to emit the
		 * IHaveTabs::addNewTab() signal when needed.
		 *
		 * @sa TFSingle.
		 */
		TFOpenableByRequest = 1 << 0,

		/** @brief There could be only one instance of this tab.
		 *
		 * By default, LeechCraft considers that each tab having the
		 * TFOpenableByRequest feature could be opened multiple times,
		 * but sometimes it doesn't make sense to have more than one tab
		 * of some class. In this case, this feature should also be
		 * present for that tab class.
		 *
		 * This feature requires the TFOpenableByRequest feature as
		 * well.
		 *
		 * @sa TFOpenableByRequest.
		 */
		TFSingle = 1 << 1,

		/** @brief The tab should be opened by default.
		 *
		 * By default, all tabs are hidden, both having TFSingle feature
		 * and lacking it. If a tab wants to be shown after LeechCraft
		 * startup until the user manually closes it, the corresponding
		 * tab class should have this feature as well.
		 */
		TFByDefault = 1 << 2,

		/** @brief The tab is to be suggested in a quick launch area.
		 *
		 * Tabs having this flag are expected to be contained by default
		 * in some kind of quick launch area like the one in the Sidebar
		 * plugin.
		 *
		 * Consider adding this flag if you think user would often open
		 * tabs of your class.
		 */
		TFSuggestOpening = 1 << 3,

		/** @brief The tab uses the standard tab close shortcut (Ctrl+W).
		 *
		 * Thus, when this tab is active, the tab close shortcut will be
		 * disabled if it matches "Ctrl+W".
		 */
		TFOverridesTabClose = 1 << 4
	};

	Q_DECLARE_FLAGS (TabFeatures, LC::TabFeature);

	/** @brief The structure describing a single tab class.
	 */
	struct TabClassInfo
	{
		/** @brief The tab class ID, which should be globally unique.
		 *
		 * This ID would be passed to the IHaveTabs::TabOpenRequested()
		 * method if the user decides to open a tab of this class.
		 */
		QByteArray TabClass_;

		/** @brief Visible name for the given tab class.
		 *
		 * The visible name is used, for example, on actions used to
		 * open tabs of this class.
		 */
		QString VisibleName_;

		/** @brief The description of the given tab class.
		 *
		 * A human-readable string with description of the purpose of
		 * the tabs belonging to this tab class.
		 */
		QString Description_;

		/** @brief The icon for the given tab class.
		 *
		 * This icon is used, for example, on actions used to open tabs
		 * of this class.
		 */
		QIcon Icon_;

		/** @brief The priority of this tab class.
		 *
		 * Refer to the documentation of IHaveTabs for the explanation
		 * of the priorities system.
		 */
		quint16 Priority_;

		/** @brief The features of this tab class.
		 *
		 * @sa TabFeature
		 */
		TabFeatures Features_;
	};

	typedef QList<TabClassInfo> TabClasses_t;
};

class QToolBar;
class QAction;

/** @brief This interface defines methods that should be implemented in
 * widgets added to the main tab widget.
 *
 * A tab may also implement the following interfaces:
 * - IRecoverableTab if it makes sense to store the tab in tab session
 *   snapshots and IWk
 * - IWkFontSettable if the tab has a QtWebKit view and wishes to support
 *   configuring the fonts used in the view by the user.
 *
 * @sa IRecoverableTab
 * @sa IWkFontSettable
 */
class Q_DECL_EXPORT ITabWidget
{
public:
	virtual ~ITabWidget () {}

	/** @brief Returns the description of the tab class of this tab.
	 *
	 * The entry must be the same as the one with the same TabClass_
	 * returned from the IHaveTabs::GetTabClasses().
	 *
	 * @return The tab class description.
	 *
	 * @sa IHavetabs::GetTabClasses()
	 */
	virtual LC::TabClassInfo GetTabClassInfo () const = 0;

	/** @brief Returns the pointer to the plugin this tab belongs to.
	 *
	 * The returned object must implement IHaveTabs and must be the one
	 * that called IHaveTabs::addNewTab() with this tab as the
	 * parameter.
	 *
	 * @return The pointer to the plugin that this tab belongs to.
	 */
	virtual QObject* ParentMultiTabs () = 0;

	/** @brief Requests to remove the tab.
	 *
	 * This method is called by LeechCraft Core (or other plugins) when
	 * this tab should be closed, for example, when user clicked on the
	 * 'x' in the tab bar. The tab may ask the user if he really wants
	 * to close the tab, for example. The tab is free to ignore the
	 * close request (in this case nothing should be done at all) or
	 * accept it by emitting IHavetabs::removeTab() signal, passing this
	 * tab widget as its parameter.
	 */
	virtual void Remove () = 0;

	/** @brief Requests tab's toolbar.
	 *
	 * This method is called to obtain the tab toolbar. In current
	 * implementation, it would be shown on top of the LeechCraft's main
	 * window.
	 *
	 * If the tab has no toolbar, 0 should be returned.
	 *
	 * @return The tab's toolbar, or 0 if there is no toolbar.
	 */
	virtual QToolBar* GetToolBar () const = 0;

	/** @brief Returns the list of QActions for the context menu of the
	 * tabbar.
	 *
	 * These actions would be shown in the context menu that would
	 * pop-up when the user right-clicks this tab's button in the
	 * tabbar.
	 *
	 * The default implementation returns an empty list.
	 *
	 * @return The list of actions for tabbar context menu.
	 */
	virtual QList<QAction*> GetTabBarContextMenuActions () const
	{
		return {};
	}

	/** @brief Returns the list of QActions to be inserted into global
	 * menu.
	 *
	 * For each key in the map (except special values, which would be
	 * defined later), the corresponding submenu would be created in
	 * LeechCraft global menu, and the corresponding list of actions
	 * would be inserted into that submenu. The submenus created would
	 * only be visible when this tab is active.
	 *
	 * There are special values for the keys:
	 * - "view" for the View menu.
	 * - "tools" for the Tools menu.
	 *
	 * The default implementation returns an empty map.
	 *
	 * @return The map with keys identifying menus and values containing
	 * lists of actions to be inserted into corresponding menus.
	 */
	virtual QMap<QString, QList<QAction*>> GetWindowMenus () const
	{
		return {};
	}

	/** @brief This method is called when this tab becomes active.
	 *
	 * The default implementation does nothing.
	 */
	virtual void TabMadeCurrent ()
	{
	}

	/** @brief This method is called when another tab becomes active.
	 *
	 * This method is called only if this tab was active before the
	 * other tab activates.
	 */
	virtual void TabLostCurrent ()
	{
	}

	/** @brief This signal is emitted by a tab when it wants to remove
	 * itself.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 */
	virtual void removeTab () = 0;

	/** @brief This signal is emitted by a tab to change its name.
	 *
	 * The name of the tab is shown in the tab bar of the tab widget. It
	 * also may be shown in other places and contexts, like in the
	 * LeechCraft title bar when the corresponding tab is active.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] name The new name of the tab with tabContents.
	 */
	virtual void changeTabName (const QString& name)
	{
		Q_UNUSED (name)
	}

	/** @brief This signal is emitted by a tab to change its icon.
	 *
	 * Null icon object may be used to clear the icon.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @param[out] icon The new icon of the tab with tabContents.
	 *
	 * @sa addNewTab().
	 */
	virtual void changeTabIcon (const QIcon& icon)
	{
		Q_UNUSED (icon)
	}

	/** @brief This signal is emitted by a tab to change its status bar text.
	 *
	 * The text set by this signal would be shown when the corresponding
	 * tab is active. To clear the status bar, this signal should be
	 * emitted with empty text.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 *
	 * @note User may choose to hide the status bar, so important
	 * information should not be presented this way.
	 *
	 * @param[out] text The new statusbar text of the tab with
	 * tabContents.
	 *
	 * @sa addNewTab().
	 */
	virtual void statusBarChanged (const QString& text)
	{
		Q_UNUSED (text)
	}

	/** @brief This signal is emitted by a tab to bring itself to the front.
	 *
	 * @note This function is expected to be a signal in subclasses.
	 */
	virtual void raiseTab ()
	{
	}
};

/** @brief Interface for plugins that have one or more tabs.
 *
 * Each plugin that may have tabs in LeechCraft should implement this
 * interface.
 *
 * Plugins implementing this interface may have one or more tabs of
 * different semantics, like chat tabs and service discovery tabs in an
 * IM or download tab and hub browse tab in a DirectConnect plugin.
 *
 * Different tabs with different semantics are said to belong to
 * different tab classes. Different tab classes may have different
 * behavior, but tabs of the same tab class are considered to be
 * semantically equivalent. For example, there may be only one opened
 * tab with the list of active downloads at a time, but there may be
 * many simultaneously opened tabs for hub browsing. Tab behavior is
 * defined by the LC::TabFeature enum.
 *
 * Different tab classes may have different priorities. The priorities
 * system is used to try to guess the most-currently-wanted tab by the
 * user. When user requests a new tab, but doesn't specify its type (for
 * example, just hits Ctrl+T), priorities of two tab classes are
 * compared: the priority of the class of the current tab and the
 * highest priority among all the tabs. If current priority plus some
 * delta is higher than maximum one, a new instance of current tab class
 * is opened, otherwise the tab with the highest priority is opened. For
 * example, if web browser tab has priority of 80, text editor — 70 and
 * search plugin — 60, and delta is 15, then if current tab is web
 * browser or search plugin, the new tab will be a web browser tab (since
 * 60 + 15 < 80), but if the current tab is text editor's one, then the
 * new tab will also be a text editor (70 + 15 > 80).
 *
 * In future implementations user may be allowed to adjust the delta and
 * priorities of different classes to his liking.
 *
 * @note You mustn't use tab-related signals before SecondInit() has
 * been called on your plugin, but you may use them in SecondInit() or
 * later.
 *
 * @sa ITabWidget, LC::TabClassInfo
 */
class Q_DECL_EXPORT IHaveTabs
{
public:
	virtual ~IHaveTabs () {}

	/** @brief Returns the list of tab classes provided by this plugin.
	 *
	 * This list must not change between different calls to this
	 * function.
	 *
	 * @note Actually, not all tab classes returned from this method
	 * have to result in a new tab being opened when called the
	 * TabOpenRequested() method. For example, the Azoth plugin returns
	 * a tab class for a fictional tab that, when passed to the
	 * TabOpenRequested() method, results in MUC join dialog appearing.
	 *
	 * @return The list of tab classes this plugin provides.
	 *
	 * @sa TabClassInfo, ITabWidget::GetTabClass(), TabOpenRequested().
	 */
	virtual LC::TabClasses_t GetTabClasses () const = 0;

	/** @brief Opens the new tab from the given tabClass.
	 *
	 * This method is called to notify the plugin that a tab of the
	 * given tabClass is requested by the user.
	 *
	 * @note Please note that the call to this method doesn't have to
	 * result in a new tab being opened. Refer to the note in
	 * GetTabClasses() documentation for more information.
	 *
	 * @param[in] tabClass The class of the requested tab, from the
	 * returned from GetTabClasses() list.
	 *
	 * @sa GetTabClasses()
	 */
	virtual void TabOpenRequested (const QByteArray& tabClass) = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::TabFeatures)

Q_DECLARE_INTERFACE (ITabWidget, "org.Deviant.LeechCraft.ITabWidget/1.0")
Q_DECLARE_INTERFACE (IHaveTabs, "org.Deviant.LeechCraft.IHaveTabs/1.0")

#endif
