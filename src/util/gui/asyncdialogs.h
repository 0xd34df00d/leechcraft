/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QFileDialog>
#include <QLineEdit>
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

	/** @brief The parameters of the file dialogs.
	 *
	 * The fields mean the same as the like-named parameters of the static
	 * `QFileDialog` functions.
	 *
	 * @ingroup GuiUtil
	 */
	struct FileDialogParams
	{
		QWidget *Parent_ = nullptr;
		QString Caption_ {};
		QString Dir_ {};
		QString Filter_ {};
		QFileDialog::Options Options_ {};
	};

	/** @brief The parameters of the directory dialog.
	 *
	 * The fields mean the same as the like-named parameters of
	 * `QFileDialog::getExistingDirectory()`.
	 *
	 * @ingroup GuiUtil
	 */
	struct DirectoryDialogParams
	{
		QWidget *Parent_ = nullptr;
		QString Caption_ {};
		QString Dir_ {};
		QFileDialog::Options Options_ = QFileDialog::ShowDirsOnly;
	};

	/** @brief Asks for a file to open without running a nested event loop.
	 *
	 * The asynchronous counterpart of `QFileDialog::getOpenFileName()`:
	 * the dialog is shown just as that function shows it, but the call
	 * returns right away, and awaiting the returned task yields what the
	 * function would return:
	 * ```cpp
	 * const auto path = co_await Util::Dialogs::GetOpenFileName ({
	 * 			.Parent_ = this,
	 * 			.Caption_ = tr ("Open image"),
	 * 			.Filter_ = tr ("Images (*.png *.jpg)"),
	 * 		});
	 * if (path.isEmpty ())
	 * 	co_return;
	 * ```
	 *
	 * @return The chosen file, or an empty string if the dialog is
	 * cancelled or is destroyed before that, as it is along with its
	 * parent.
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API Task<QString> GetOpenFileName (const FileDialogParams& params);

	/** @brief `GetOpenFileName()` for several files, as `QFileDialog::getOpenFileNames()`.
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API Task<QStringList> GetOpenFileNames (const FileDialogParams& params);

	/** @brief `GetOpenFileName()` for a file to save to, as `QFileDialog::getSaveFileName()`.
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API Task<QString> GetSaveFileName (const FileDialogParams& params);

	/** @brief `GetOpenFileName()` for a directory, as `QFileDialog::getExistingDirectory()`.
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API Task<QString> GetExistingDirectory (const DirectoryDialogParams& params);

	/** @brief The parameters of the item dialog.
	 *
	 * The fields mean the same as the like-named parameters of
	 * `QInputDialog::getItem()`.
	 *
	 * @ingroup GuiUtil
	 */
	struct ItemDialogParams
	{
		QWidget *Parent_ = nullptr;
		QString Title_ {};
		QString Label_ {};
		QStringList Items_ {};
		int Current_ = 0;
		bool Editable_ = true;
	};

	/** @brief Asks to choose an item without running a nested event loop.
	 *
	 * The asynchronous counterpart of `QInputDialog::getItem()`: the
	 * dialog is shown just as that function shows it, but the call returns
	 * right away, and awaiting the returned task yields the choice:
	 * ```cpp
	 * const auto format = co_await Util::Dialogs::GetItem ({
	 * 			.Parent_ = this,
	 * 			.Title_ = tr ("Export"),
	 * 			.Label_ = tr ("Choose the format:"),
	 * 			.Items_ = formats,
	 * 			.Editable_ = false,
	 * 		});
	 * if (!format)
	 * 	co_return;
	 * ```
	 *
	 * @return The text of the chosen item, or nothing if the dialog is
	 * cancelled or is destroyed before that, as it is along with its
	 * parent.
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API Task<std::optional<QString>> GetItem (const ItemDialogParams& params);

	/** @brief The parameters of the text dialog.
	 *
	 * The fields mean the same as the like-named parameters of
	 * `QInputDialog::getText()`.
	 *
	 * @ingroup GuiUtil
	 */
	struct TextDialogParams
	{
		QWidget *Parent_ = nullptr;
		QString Title_ {};
		QString Label_ {};
		QLineEdit::EchoMode Echo_ = QLineEdit::Normal;
		QString Text_ {};
	};

	/** @brief Asks for a line of text without running a nested event loop.
	 *
	 * The asynchronous counterpart of `QInputDialog::getText()`: the
	 * dialog is shown just as that function shows it, but the call returns
	 * right away, and awaiting the returned task yields the text:
	 * ```cpp
	 * const auto name = co_await Util::Dialogs::GetText ({
	 * 			.Parent_ = this,
	 * 			.Title_ = tr ("Rename"),
	 * 			.Label_ = tr ("Enter the new name:"),
	 * 			.Text_ = oldName,
	 * 		});
	 * if (!name)
	 * 	co_return;
	 * ```
	 *
	 * @return The entered text, or nothing if the dialog is cancelled or
	 * is destroyed before that, as it is along with its parent.
	 *
	 * @ingroup GuiUtil
	 */
	UTIL_GUI_API Task<std::optional<QString>> GetText (const TextDialogParams& params);
}
