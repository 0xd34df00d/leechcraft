/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QMessageBox>
#include <util/threads/coro/task.h>
#include "guiconfig.h"

namespace LC::Util::Dialogs
{
	/** @brief Shows a dialog modally without running a nested event loop.
	 *
	 * The asynchronous counterpart of `QDialog::exec()`: the dialog is
	 * shown just as `exec()` shows it, but the call returns right away,
	 * and awaiting the returned task yields what `exec()` would return.
	 *
	 * Example use:
	 * ```cpp
	 * SubmitToDialog dia { this };
	 * if (co_await Util::Dialogs::Exec (dia) != QDialog::Accepted)
	 * 	co_return;
	 * ```
	 *
	 * A dialog local to the awaiting coroutine shall not be deleted by its
	 * parent: leave it parentless, or parent it to the object whose
	 * `CoroContext` the coroutine is registered with.
	 *
	 * @param dia The dialog to show.
	 * @return The result code of the dialog, or `QDialog::Rejected` if it
	 * is destroyed before finishing.
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API Task<int> Exec (QDialog& dia);

	/** @brief Shows a message box modally without running a nested event loop.
	 *
	 * The asynchronous counterpart of the static `QMessageBox` functions:
	 * the parameters mean the same and the box is shown just as they show
	 * it, but the call returns right away. Awaiting the returned task
	 * yields what the static function would return, and is only needed to
	 * wait for the answer:
	 * ```cpp
	 * if (co_await Util::Dialogs::Question (this, title, text) != QMessageBox::Yes)
	 * 	co_return;
	 *
	 * Util::Dialogs::Warning (this, title, text);
	 * ```
	 *
	 * @return The button the box was dismissed with, or
	 * `QMessageBox::NoButton` if the box is destroyed before that, as it
	 * is along with its @p parent.
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API Task<QMessageBox::StandardButton> ShowMessageBox (QMessageBox::Icon icon,
			QWidget *parent, const QString& title, const QString& text,
			QMessageBox::StandardButtons buttons = QMessageBox::Ok,
			QMessageBox::StandardButton defaultButton = QMessageBox::NoButton);

	/** @brief `ShowMessageBox()` with the icon and the default buttons of `QMessageBox::information()`.
	 *
	 * @ingroup GuiUtil
	 */
	inline Task<QMessageBox::StandardButton> Information (QWidget *parent, const QString& title, const QString& text,
			QMessageBox::StandardButtons buttons = QMessageBox::Ok,
			QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
	{
		return ShowMessageBox (QMessageBox::Information, parent, title, text, buttons, defaultButton);
	}

	/** @brief `ShowMessageBox()` with the icon and the default buttons of `QMessageBox::question()`.
	 *
	 * @ingroup GuiUtil
	 */
	inline Task<QMessageBox::StandardButton> Question (QWidget *parent, const QString& title, const QString& text,
			QMessageBox::StandardButtons buttons = QMessageBox::Yes | QMessageBox::No,
			QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
	{
		return ShowMessageBox (QMessageBox::Question, parent, title, text, buttons, defaultButton);
	}

	/** @brief `ShowMessageBox()` with the icon and the default buttons of `QMessageBox::warning()`.
	 *
	 * @ingroup GuiUtil
	 */
	inline Task<QMessageBox::StandardButton> Warning (QWidget *parent, const QString& title, const QString& text,
			QMessageBox::StandardButtons buttons = QMessageBox::Ok,
			QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
	{
		return ShowMessageBox (QMessageBox::Warning, parent, title, text, buttons, defaultButton);
	}

	/** @brief `ShowMessageBox()` with the icon and the default buttons of `QMessageBox::critical()`.
	 *
	 * @ingroup GuiUtil
	 */
	inline Task<QMessageBox::StandardButton> Critical (QWidget *parent, const QString& title, const QString& text,
			QMessageBox::StandardButtons buttons = QMessageBox::Ok,
			QMessageBox::StandardButton defaultButton = QMessageBox::NoButton)
	{
		return ShowMessageBox (QMessageBox::Critical, parent, title, text, buttons, defaultButton);
	}
}
