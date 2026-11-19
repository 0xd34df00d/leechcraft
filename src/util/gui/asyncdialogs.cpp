/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "asyncdialogs.h"
#include <algorithm>
#include <QInputDialog>
#include <QPointer>
#include <QPushButton>
#include <QUrl>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/threads/coro/signal.h>

namespace LC::Util::Dialogs
{
	namespace
	{
		Task<int> AwaitResult (QPointer<QDialog> dia)
		{
			const auto code = co_await Signal { *dia, &QDialog::finished };
			// A `QMessageBox` closed through its window sets its result only after emitting `finished (0)`,
			// while a `WA_DeleteOnClose` dialog is already deleted by the time the coroutine is resumed.
			co_return dia ? dia->result () : code.value_or (QDialog::Rejected);
		}

		void SetDefaultButton (QMessageBox& box, QMessageBox::StandardButton defaultButton)
		{
			if (defaultButton != QMessageBox::NoButton)
			{
				box.setDefaultButton (defaultButton);
				return;
			}

			// The static `QMessageBox` functions fall back to the first button with the accept role.
			const auto& buttons = box.buttons ();
			const auto acceptPos = std::ranges::find (buttons, QMessageBox::AcceptRole,
					[&box] (QAbstractButton *button) { return box.buttonRole (button); });
			if (acceptPos != buttons.end ())
				box.setDefaultButton (qobject_cast<QPushButton*> (*acceptPos));
		}

		Task<QMessageBox::StandardButton> AwaitAnswer (QPointer<QMessageBox> box)
		{
			co_await Exec (*box);
			if (!box)
				co_return QMessageBox::NoButton;

			box->deleteLater ();
			co_return box->standardButton (box->clickedButton ());
		}

		QFileDialog* MakeFileDialog (const FileDialogParams& params, QFileDialog::FileMode mode)
		{
			const auto dia = new QFileDialog { params.Parent_, params.Caption_, params.Dir_, params.Filter_ };
			dia->setFileMode (mode);
			dia->setOptions (params.Options_);
			dia->setSupportedSchemes ({ "file"_qs });
			return dia;
		}

		Task<QList<QUrl>> AwaitUrls (QPointer<QFileDialog> dia)
		{
			const auto code = co_await Exec (*dia);
			if (!dia)
				co_return QList<QUrl> {};

			dia->deleteLater ();
			co_return code == QDialog::Accepted ? dia->selectedUrls () : QList<QUrl> {};
		}

		Task<QString> AwaitFileName (QPointer<QFileDialog> dia)
		{
			const auto url = (co_await AwaitUrls (std::move (dia))).value (0);
			co_return url.isLocalFile () || url.isEmpty () ? url.toLocalFile () : url.toString ();
		}

		Task<QStringList> AwaitFileNames (QPointer<QFileDialog> dia)
		{
			const auto urls = co_await AwaitUrls (std::move (dia));
			co_return Map (urls, [] (const QUrl& url) { return url.toString (QUrl::PreferLocalFile); });
		}

		Task<std::optional<QString>> AwaitTextValue (QPointer<QInputDialog> dia)
		{
			const auto code = co_await Exec (*dia);
			if (!dia)
				co_return {};

			dia->deleteLater ();
			co_return code == QDialog::Accepted ? std::optional { dia->textValue () } : std::nullopt;
		}
	}

	Task<int> Exec (QDialog& dia)
	{
		dia.setModal (true);
		dia.show ();
		return AwaitResult (&dia);
	}

	Task<QMessageBox::StandardButton> ShowMessageBox (QMessageBox::Icon icon,
			QWidget *parent, const QString& title, const QString& text,
			QMessageBox::StandardButtons buttons, QMessageBox::StandardButton defaultButton)
	{
		const auto box = new QMessageBox { icon, title, text, buttons, parent };
		SetDefaultButton (*box, defaultButton);
		return AwaitAnswer (box);
	}

	Task<QString> GetOpenFileName (const FileDialogParams& params)
	{
		return AwaitFileName (MakeFileDialog (params, QFileDialog::ExistingFile));
	}

	Task<QStringList> GetOpenFileNames (const FileDialogParams& params)
	{
		return AwaitFileNames (MakeFileDialog (params, QFileDialog::ExistingFiles));
	}

	Task<QString> GetSaveFileName (const FileDialogParams& params)
	{
		const auto dia = MakeFileDialog (params, QFileDialog::AnyFile);
		dia->setAcceptMode (QFileDialog::AcceptSave);
		return AwaitFileName (dia);
	}

	Task<QString> GetExistingDirectory (const DirectoryDialogParams& params)
	{
		const FileDialogParams fileParams
		{
			.Parent_ = params.Parent_,
			.Caption_ = params.Caption_,
			.Dir_ = params.Dir_,
			.Options_ = params.Options_,
		};
		return AwaitFileName (MakeFileDialog (fileParams, QFileDialog::Directory));
	}

	Task<std::optional<QString>> GetItem (const ItemDialogParams& params)
	{
		const auto dia = new QInputDialog { params.Parent_ };
		dia->setWindowTitle (params.Title_);
		dia->setLabelText (params.Label_);
		dia->setComboBoxItems (params.Items_);
		dia->setTextValue (params.Items_.value (params.Current_));
		dia->setComboBoxEditable (params.Editable_);
		return AwaitTextValue (dia);
	}

	Task<std::optional<QString>> GetText (const TextDialogParams& params)
	{
		const auto dia = new QInputDialog { params.Parent_ };
		dia->setWindowTitle (params.Title_);
		dia->setLabelText (params.Label_);
		dia->setTextValue (params.Text_);
		dia->setTextEchoMode (params.Echo_);
		return AwaitTextValue (dia);
	}
}
