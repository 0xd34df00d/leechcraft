/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <Qt>

class QDockWidget;
class QToolBar;
class QWidget;
class QKeySequence;
class QMenu;

/** @brief This interface is used for manipulating the main window.
 *
 * All the interaction with LeechCraft main window should be done
 * through this interface.
 */
class Q_DECL_EXPORT IMWProxy
{
public:
	enum WidgetArea
	{
		WALeft,
		WARight,
		WABottom
	};

	virtual ~IMWProxy () {}

	struct DockWidgetParams
	{
		Qt::DockWidgetArea Area_ = Qt::NoDockWidgetArea;
		std::optional<QByteArray> SizeContext_ = {};
	};

	/** @brief Adds the given dock widget to the main window
	 *
	 * The action for toggling the visibility of this dock widget is
	 * also added to the corresponding menus by default. The
	 * ToggleViewActionVisiblity() method could be used to change that.
	 *
	 * @param[in] widget The dock widget to add.
	 * @param[in] params The parameters of the newly added dock widget.
	 *
	 * @sa AssociateDockWidget(), ToggleViewActionVisiblity()
	 */
	virtual void AddDockWidget (QDockWidget *widget, const DockWidgetParams& params) = 0;

	/** @brief Connects the given dock widget with the given tab.
	 *
	 * This function associates the given dock widget with the given tab
	 * widget so that the dock widget is only visible when the tab is
	 * current tab.
	 *
	 * A dock widget may be associated with only one tab widget. Calling
	 * this function repeatedly will override older associations.
	 *
	 * @param[in] dock The dock widget to associate.
	 * @param[in] tab The tab widget for which the dock widget should be
	 * active.
	 *
	 * @sa AddDockWidget()
	 */
	virtual void AssociateDockWidget (QDockWidget *dock, QWidget *tab) = 0;

	/** @brief Sets the visibility of the previously added dock widget.
	 *
	 * This function sets the visibility of the given \em dock (which
	 * should be a dock widget previously added via AddDockWidget()).
	 * If \em dock has been associated with a tab via
	 * AssociateDockWidget(), calling this function makes sure that the
	 * visibility of the dock \em dock will be equal to \em visible.
	 *
	 * @note This function should be called after AssociateDockWidget()
	 * if the later is called at all.
	 *
	 * @param[in] dock The dock widget previously added via
	 * AddDockWidget().
	 * @param[in] visible The visibility of the dock widget.
	 *
	 * @sa AddDockWidget(), AssociateDockWidget()
	 */
	virtual void SetDockWidgetVisibility (QDockWidget *dock, bool visible) = 0;

	/** @brief Toggles the visibility of the toggle view action.
	 *
	 * By default all newly added dock widgets have their toggle view
	 * actions shown.
	 *
	 * @param[in] widget The widget for which to update the toggle
	 * action visibility.
	 * @param[in] visible Whether the corresponding action should be
	 * visible.
	 */
	virtual void ToggleViewActionVisiblity (QDockWidget *widget, bool visible) = 0;

	/** @brief Sets the visibility action shortcut of the given widget.
	 *
	 * @param[in] widget The widget for which the visibility action
	 * shortcut.
	 * @param[in] seq The widget's visibility action shortcut sequence.
	 */
	virtual void SetViewActionShortcut (QDockWidget *widget, const QKeySequence& seq) = 0;

	/** @brief Toggles the visibility of the main window.
	 */
	virtual void ToggleVisibility () = 0;

	/** @brief Show/raise main window
	 */
	virtual void ShowMain () = 0;

	/** @brief Returns the main LeechCraft menu.
	 *
	 * @return The main LeechCraft menu.
	 *
	 * @sa HideMainMenu()
	 */
	virtual QMenu* GetMainMenu () = 0;

	/** @brief Hides the main LeechCraft menu.
	 *
	 * Calling this function hides the main LeechCraft menu in the
	 * tabbar. There is no way of showing it back again after that. The
	 * menu is still accessible via GetMainMenu() and can be shown via
	 * other means.
	 *
	 * @sa GetMainMenu().
	 */
	virtual void HideMainMenu () = 0;
};

Q_DECLARE_INTERFACE (IMWProxy, "org.Deviant.LeechCraft.IMWProxy/1.0")
