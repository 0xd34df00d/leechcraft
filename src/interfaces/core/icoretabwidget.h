/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#ifndef INTERFACES_CORE_ICORETABWIDGET_H
#define INTERFACES_CORE_ICORETABWIDGET_H

#include <QTabBar>
#include <QVariant>

class QObject;
class QWidget;
class QIcon;
class QMenu;

/** @brief This interface is used to represent LeechCraft's core tab widget
 *
 * This interface is for communication with the core tab widget.
 *
 * Object returned by the GetObject() function emits these signals:
 * - tabWasInserted (int index) after the new tab was inserted.
 */
class ICoreTabWidget
{
public:
	virtual ~ICoreTabWidget () {}

	/** @brief Returns the pointer to tab widget as a QObject.
	 *
	 * You can connect to signals of the core tab widget with the use of
	 * this function, for example.
	 *
	 * @return The core tab widget as a QObject.
	*/
	virtual QObject* GetObject () = 0;

	/** @brief Returns the number of widgets associated with tabs.
	 *
	 * @return pages count.
	*/
	virtual int WidgetCount () const = 0;

	/** @brief Returns the tab page at index position index or 0.
	 * if the index is out of range.
	 *
	 * @param[in] index tab index.
	 * @return the page at inted position.
	 */
	virtual QWidget* Widget (int index) const = 0;

	/** @brief Returns the index of the given page.
	 *
	 * @param[in] page page to find.
	 * @return the index of that page, or -1 if not found.
	 */
	virtual int IndexOf (QWidget *page) const = 0;

	/** @brief Returns the tab menu for the given tab index.
	 *
	 * Ownership of the menu goes to the caller — it's his responsibility
	 * to delete the menu when done.
	 *
	 * @param[in] index tab index.
	 * @return tab menu for that index, or null if index is invalid.
	 */
	virtual QMenu* GetTabMenu (int index) = 0;

	/** @brief Returns the list of actions witch always shows in context menu.
	 * of the tab
	 *
	 * @return list of actions.
	 */
	virtual QList<QAction*> GetPermanentActions () const = 0;

	/** @brief Returns the data of the tab at position index, or a null variant
	 * if index is out of range.
	 *
	 * @param[in] index tab index.
	 * @return the data of specified tab.
	 */
	virtual QVariant TabData (int index) const = 0;

	/** @brief Sets the data of the tab at position index to data.
	 *
	 * @param[in] index tab index.
	 * @param[in] data new tab data.
	 */
	virtual void SetTabData (int index, QVariant data) = 0;

	/** @brief Returns the text of the tab at position index,
	 * or an empty string if index is out of range.
	 *
	 * @param[in] index tab index.
	 * @return specified tab text.
	 */
	virtual QString TabText (int index) const = 0;

	/** @brief Sets the text of the tab at position index to text.
	 * of the tabs
	 *
	 * @param[in] index tab index.
	 * @param[in] text new tab text.
	 */
	virtual void SetTabText (int index, const QString& text) = 0;

	/** @brief Returns the icon of the tab at position index,
	 * or a null icon if index is out of range.
	 *
	 * @param[in] index tab index.
	 * @return specified tab icon.
	 */
	virtual QIcon TabIcon (int index) const = 0;

	/** @brief Returns the widget set a tab index and position
	 * or 0 if one is not set.
	 *
	 * @param[in] index tab index.
	 * @param[in] position position of widget.
	 * @return widget in tab.
	 */
	virtual QWidget* TabButton (int index, QTabBar::ButtonPosition positioin) const = 0;

	/** @brief Returns the position of close button
	 *
	 * @return close button position.
	 */
	virtual QTabBar::ButtonPosition GetCloseButtonPosition () const = 0;

	/** @brief Sets tab closable.
	 *
	 * @param[in] index tab index.
	 * @param[in] closable set tab closable.
	 * @param[in] closeButton set close button.
	 */
	virtual void SetTabClosable (int index, bool closable, QWidget *closeButton = 0) = 0;

	/** @brief Returns the index of the tab bar's visible tab.
	 *
	 * @return tab index.
	 */
	virtual int CurrentIndex () const = 0;

	/** @brief Moves the item at index position from to index position to.
	 *
	 * @param[in] from source position.
	 * @param[in] to destination position.
	 */
	virtual void MoveTab (int from, int to) = 0;

	/** @brief Sets the current tab index to specified index.
	 *
	 * @param[in] index new tab index.
	 */
	virtual void setCurrentIndex (int index) = 0;

	/** @brief Sets the current tab index to specified associated widget.
	 *
	 * @param[in] widget page.
	 */
	virtual void setCurrentWidget (QWidget *widget) = 0;

	/** @brief This signal is emitted after new tab was inserted.
	 *
	 * @param[out] index The index of new tab.
	 */
	virtual void tabInserted (int index) = 0;
};

Q_DECLARE_INTERFACE (ICoreTabWidget, "org.Deviant.LeechCraft.ICoreTabWidget/1.0");

#endif
