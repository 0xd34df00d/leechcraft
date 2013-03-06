/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
	 * LeechCraft::TabRecoverInfo::Data_ member.
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

namespace LeechCraft
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
 * closed tab), the RecoverTabs method will be called by a tab
 * session manager plugin to recover the needed tabs.
 *
 * c
 *
 * @sa IRecoverableTab, LeechCraft::TabRecoverInfo, IHaveTabs
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
	 * according to LeechCraft::TabRecoverInfo::Data_, set the dynamic
	 * properties of the tab (via QObject::setProperty()) according to
	 * LeechCraft::TabRecoverInfo::DynProperties_ list, and only then
	 * emit the IHaveTabs::addNewTab() signal.
	 *
	 * @note Please note that it's very important to emit the tab via
	 * the addNewTab() signal only _after_ the tab's dynamic properties
	 * are restored.
	 */
	virtual void RecoverTabs (const QList<LeechCraft::TabRecoverInfo>& infos) = 0;
};

Q_DECLARE_INTERFACE (IRecoverableTab, "org.Deviant.LeechCraft.IRecoverableTab/1.0");
Q_DECLARE_INTERFACE (IHaveRecoverableTabs, "org.Deviant.LeechCraft.IHaveRecoverableTabs/1.0");

#endif
