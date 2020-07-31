/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "gwoptionsdialog.h"
#include <QPushButton>
#include <QMessageBox>
#include "regformhandlerwidget.h"

namespace LC
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
