/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QString>
#include <QList>

class QAction;
class QIcon;

/** @brief Interface for accessing LeechCraft-wide icons manager.
 *
 * The icon theme manager provides access to actions icon sets managed
 * globally in LeechCraft.
 *
 * The icons are identified by their Freedesktop.org-like name, and they
 * should be contained at least in the KDE's Oxygen icon theme.
 *
 * Icons may be retrieved via the GetIcon() method, and actions (QAction
 * subclasses) can be automatically set with a proper icon by setting
 * their \em ActionIcon dynamic property to the name of the desired icon.
 * This way they will also be automatically updated if the iconset
 * changes.
 *
 * Please note that the \em ActionIcon property should be set before
 * QAction (or its parent) widget is exposed to the rest oF LeechCraft
 * (for example, a menu embedded into a ITabWidget-implementing tab, or
 * an ITabWidget added to LeechCrat windows). Otherwise, UpdateIconset()
 * should be called on the actions for them to be registered.
 */
class IIconThemeManager
{
protected:
	virtual ~IIconThemeManager () {}
public:
	/** @brief Returns the current theme's icon for the given on and off
	 * states.
	 *
	 * The name should match the Freedesktop.org's icons naming scheme
	 * and be contained at least in the KDE's Oxygen iconset.
	 *
	 * @param[in] on The name of the icon in the "on" state.
	 * @param[in] off The name of the icon in the "off" state, if any.
	 * @return The QIcon object created from image files which could be
	 * obtained via GetIconPath().
	 *
	 * @sa GetIconPath
	 */
	virtual QIcon GetIcon (const QString& on, const QString& off = QString ()) = 0;

	/** @brief Updates the icons of the given actions.
	 *
	 * This function sets or updates the icons of \em actions according
	 * to the current iconset. This function also registers the actions
	 * so that they are automatically updated when the iconset changes,
	 * analogously to ManageWidget().
	 *
	 * @param[in] actions The list of actions to update.
	 *
	 * @sa ManageWidget()
	 */
	virtual void UpdateIconset (const QList<QAction*>& actions) = 0;

	/** @brief Watches the given widget recursively and its child actions.
	 *
	 * This function merely installs the event filter on the given widget
	 * to watch for new actions or action changes.
	 *
	 * @param[in] widget The widget to manage.
	 */
	virtual void ManageWidget (QWidget *widget) = 0;

	virtual QIcon GetPluginIcon () = 0;

	virtual QIcon GetPluginIcon (const QString& name) = 0;
};

Q_DECLARE_INTERFACE (IIconThemeManager, "org.LeechCraft.IIconThemeManager/1.0")
