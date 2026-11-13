/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "asyncdialogs.h"
#include <algorithm>
#include <QPointer>
#include <QPushButton>
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
}
