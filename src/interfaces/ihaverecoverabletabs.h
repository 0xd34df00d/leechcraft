/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_IHAVERECOVERABLETABS_H
#define INTERFACES_IHAVERECOVERABLETABS_H
#include <QList>
#include <QByteArray>
#include <QVariant>

class QWidget;
class QIcon;

/** @brief Interface for a single tab that may be recovered.
 *
 * This interface should be implemented by tabs that may be recovered
 * across LeechCraft runs or after closing. For example, a web browser
 * tab with a web page or a chat window may be recovered either on next
 * LeechCraft start or after being closed.
 *
 * The tab's state should be returned by the GetTabRecoverData()
 * function. Tab name and icon for the restore dialog or menu should be
 * returned by the GetTabRecoverName() and GetTabRecoverIcon() functions
 * respectively. Whenever tab state changes, it should emit the
 * tabRecoverDataChanged() signal to notify tab session managers about
 * the update.
 *
 * @sa IHaveRecoverableTabs
 */
class Q_DECL_EXPORT IRecoverableTab
{
public:
	virtual ~IRecoverableTab () {}

	/** @brief Returns the serialized state of the tab.
	 *
	 * This state will later be passed to the
	 * IHaveRecoverableTabs::RecoverTabs() as the
	 * LC::TabRecoverInfo::Data_ member.
	 */
	virtual QByteArray GetTabRecoverData () const = 0;

	/** @brief Returns the user-readable name of the tab.
	 *
	 * The return value of this function will be used in the tab restore
	 * dialog or tab reopen menu.
	 *
	 * For example, a web browser may want to return something like
	 * "Google — http://google.com".
	 */
	virtual QString GetTabRecoverName () const = 0;

	/** @brief Returns the icon of this tab.
	 *
	 * The return value of this function will be used in the tab restore
	 * dialog or tab reopen menu.
	 *
	 * For example, a web browser may want to return web site's favicon.
	 */
	virtual QIcon GetTabRecoverIcon () const = 0;
protected:
	/** @brief Notifies that tab state's changed.
	 *
	 * This signal should be emitted whenever the tab state changes so
	 * that tab session managers could have a chance to update their
	 * saved state.
	 *
	 * @note This function is expected to be a signal.
	 */
	virtual void tabRecoverDataChanged () = 0;
};

namespace LC
{
	typedef QList<QPair<QByteArray, QVariant>> DynPropertiesList_t;

	/** @brief Keeps the tab state between runs.
	 *
	 * The tab state consists of two parts:
	 *  - plugin- and tab-specific restore information are kept in the
	 *    Data_ member.
	 *  - dynamic properties that may be set by other plugins (like
	 *    PinTab) are kept in the DynProperties_ member.
	 *
	 * @note Only dynamic properties with names starting with the
	 * "SessionData/" string will be saved.
	 */
	struct TabRecoverInfo
	{
		/** @brief The tab-specific restore data.
		 *
		 * For example, a web browser would store the URL of the page
		 * in this member, as well as scroll position etc.
		 */
		QByteArray Data_;

		/** @brief Dynamic properties list from other plugins.
		 *
		 * @note Only dynamic properties with names starting with the
		 * "SessionData/" string will be saved.
		 */
		DynPropertiesList_t DynProperties_;
	};
}

/** @brief Interface for plugins that can recover tabs after restart.
 *
 * This interface should be implemented by plugins for which it makes
 * sense to recover tabs in some way: either after restart or un-close,
 * for instance. For example, a web browser or a media player may wish
 * to implement this interface.
 *
 * First, tabs which wish to be saved between runs should implement
 * the IRecoverableTab interface. If a session manager plugin (like
 * TabSessManager) is installed, then it will query the tabs regarding
 * their state via that interface and save that information.
 *
 * After restarting LeechCraft (or when requesting reopening a recently
 * closed tab), the RecoverTabs() method will be called by a tab session
 * manager plugin to recover the needed tabs.
 *
 * @sa IRecoverableTab, LC::TabRecoverInfo, IHaveTabs
 */
class Q_DECL_EXPORT IHaveRecoverableTabs
{
public:
	virtual ~IHaveRecoverableTabs () {}

	/** @brief Recovers the tabs according to the infos list.
	 *
	 * This method should recover the tabs according to the information
	 * contained in the infos list. That is, for each tab recover info
	 * in that list it should create the tab, recover the tab state
	 * according to LC::TabRecoverInfo::Data_, set the dynamic
	 * properties of the tab (via QObject::setProperty()) according to
	 * LC::TabRecoverInfo::DynProperties_ list, and only then
	 * emit the IHaveTabs::addNewTab() signal.
	 *
	 * @note Please note that it's very important to emit the tab via
	 * the addNewTab() signal only \em after the tab's dynamic properties
	 * are restored.
	 */
	virtual void RecoverTabs (const QList<LC::TabRecoverInfo>& infos) = 0;

	/** @brief Checks if there is a tab similar to the one defined by \em data.
	 *
	 * The \em data is guaranteed to be obtained from a tab belonging
	 * to the plugin being queried. That is, there is no need to perform any
	 * checks for the tab to be belonging to the plugin.
	 *
	 * A standard implementation is provided for the convenience in the
	 * form of the StandardSimilarImpl() function.
	 *
	 * @param[in] data The tab recover data previously obtained from
	 * IRecoverableTab::GetTabRecoverData()
	 * @param[in] existing The list of existing tabs, provided for convenience.
	 * @return Whether the tab similar to the one defined by \em data
	 * exists already.
	 *
	 * @sa StandardSimilarImpl()
	 */
	virtual bool HasSimilarTab (const QByteArray& data,
			const QList<QByteArray>& existing) const = 0;
protected:
	/** @brief A standard implementation of the HasSimilarTab() function.
	 *
	 * This function is suitable for calling from HasSimilarTab() given
	 * an additional functor \em f, which should return some equality
	 * comparable type. If <code>f(data)</code> is equal to
	 * <code>f(e)</code> for some \em e in \em existing, then this
	 * function returns \em true.
	 *
	 * @tparam T The type of the functor \em f.
	 *
	 * @param[in] data The tab recover data previously obtained from
	 * IRecoverableTab::GetTabRecoverData()
	 * @param[in] existing The list of existing tabs, provided for convenience.
	 * @param[in] f A functor returning some equality comparable type.
	 * @return Whether the tab similar to the one defined by \em data
	 * exists already.
	 *
	 * @sa HasSimilarTab()
	 */
	template<typename T>
	static bool StandardSimilarImpl (const QByteArray& data,
			const QList<QByteArray>& existing, const T& f)
	{
		const auto& thisData = f (data);
		return std::any_of (existing.begin (), existing.end (),
				[&thisData, &f] (const QByteArray& other) { return thisData == f (other); });
	}
};

Q_DECLARE_INTERFACE (IRecoverableTab, "org.Deviant.LeechCraft.IRecoverableTab/1.0")
Q_DECLARE_INTERFACE (IHaveRecoverableTabs, "org.Deviant.LeechCraft.IHaveRecoverableTabs/1.0")

#endif
