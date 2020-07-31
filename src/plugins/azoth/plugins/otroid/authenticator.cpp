/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "authenticator.h"
#include <QInputDialog>
#include <QMessageBox>
#include <interfaces/azoth/iclentry.h>
#include "initiateauthdialog.h"

namespace LC
{
namespace Azoth
{
namespace OTRoid
{
	Authenticator::Authenticator (ICLEntry *entry)
	: QObject { entry->GetQObject () }
	, Entry_ { entry }
	, HrId_ { Entry_->GetHumanReadableID () }
	, Name_ { Entry_->GetEntryName () }
	{
	}

	Authenticator::~Authenticator ()
	{
		emit destroyingAuth (Entry_);
	}

	void Authenticator::AskFor (SmpMethod method, const QString& question, ConnContext *context)
	{
		QString str;
		switch (method)
		{
		case SmpMethod::Question:
			str = tr ("%1 (%2) wants to authenticate with you via a question. The question is:")
					.arg (Name_)
					.arg (HrId_);
			str += " <em>" + question + "</em>";
			break;
		case SmpMethod::SharedSecret:
			str = tr ("%1 (%2) wants to authenticate with you via a shared secret.")
					.arg (Name_)
					.arg (HrId_);
			break;
		}

		const auto& reply = QInputDialog::getText (nullptr,
				tr ("OTR authentication"),
				str);
		if (reply.isEmpty ())
		{
			emit abortSmp (context);
			deleteLater ();
			return;
		}

		emit gotReply (method, reply, context);
	}

	void Authenticator::Initiate ()
	{
		InitiateAuthDialog dia { Entry_ };
		if (dia.exec () != QDialog::Accepted)
		{
			deleteLater ();
			return;
		}

		emit initiateRequested (Entry_, dia.GetMethod (), dia.GetQuestion (), dia.GetAnswer ());
	}

	void Authenticator::Failed ()
	{
		QMessageBox::critical (nullptr,
				tr ("OTR authentication"),
				tr ("Failed to authenticate %1 (%2).")
						.arg ("<em>" + Name_ + "</em>")
						.arg ("<em>" + HrId_ + "</em>"));
	}

	void Authenticator::Cheated ()
	{
		QMessageBox::critical (nullptr,
				tr ("OTR authentication"),
				tr ("Failed to authenticate %1 (%2): cheating detected.")
						.arg ("<em>" + Name_ + "</em>")
						.arg ("<em>" + HrId_ + "</em>"));
	}

	void Authenticator::Success ()
	{
		QMessageBox::information (nullptr,
				tr ("OTR authentication"),
				tr ("Congratulations! Contact %1 (%2) authenticated successfully!")
						.arg ("<em>" + Name_ + "</em>")
						.arg ("<em>" + HrId_ + "</em>"));
	}
}
}
}
