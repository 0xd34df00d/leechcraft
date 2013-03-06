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

#include "gwoptionsdialog.h"
#include <QPushButton>
#include <QMessageBox>
#include "regformhandlerwidget.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	GWOptionsDialog::GWOptionsDialog (QXmppClient *client, const QString& to, QWidget *parent)
	: QDialog (parent)
	, RegForm_ (new RegFormHandlerWidget (client))
	{
		Ui_.setupUi (this);
		qobject_cast<QVBoxLayout*> (layout ())->insertWidget (0, RegForm_);

		connect (RegForm_,
				SIGNAL (completeChanged ()),
				this,
				SLOT (handleCompleteChanged ()));

		disconnect (Ui_.ButtonBox_,
				SIGNAL (accepted ()),
				this,
				SLOT (accept ()));
		connect (Ui_.ButtonBox_,
				SIGNAL (accepted ()),
				this,
				SLOT (sendRegistration ()));

		RegForm_->SendRequest (to);
	}

	void GWOptionsDialog::sendRegistration ()
	{
		connect (RegForm_,
				SIGNAL (successfulReg ()),
				this,
				SLOT (accept ()));
		connect (RegForm_,
				SIGNAL (regError (QString)),
				this,
				SLOT (handleError (QString)));

		RegForm_->Register ();
		RegForm_->setEnabled (false);
	}

	void GWOptionsDialog::handleError (const QString& error)
	{
		QMessageBox::critical (this,
				"LeechCraft",
				tr ("Error updating gateway information: %1.")
					.arg (error));
		reject ();
	}

	void GWOptionsDialog::handleCompleteChanged ()
	{
		Ui_.ButtonBox_->button (QDialogButtonBox::Ok)->setEnabled (RegForm_->IsComplete ());
	}
}
}
}
