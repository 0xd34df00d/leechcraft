/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "callchatwidget.h"
#include <QTimer>

namespace LC
{
namespace Azoth
{
	CallChatWidget::CallChatWidget (QObject *call, QWidget *parent)
	: QWidget { parent }
	, CallObject_ { call }
	, Call_ { qobject_cast<IMediaCall*> (call) }
	{
		Ui_.setupUi (this);

		Ui_.StatusLabel_->setText (tr ("Initializing..."));

		if (Call_->GetDirection () == IMediaCall::DOut)
			Ui_.AcceptButton_->hide ();

		connect (call,
				SIGNAL (destroyed ()),
				this,
				SLOT (scheduleDelete ()));

		connect (call,
				SIGNAL (stateChanged (LC::Azoth::IMediaCall::State)),
				this,
				SLOT (handleStateChanged (LC::Azoth::IMediaCall::State)));
	}

	void CallChatWidget::handleStateChanged (IMediaCall::State state)
	{
		switch (state)
		{
		case IMediaCall::SConnecting:
			Ui_.StatusLabel_->setText (tr ("Connecting..."));
			break;
		case IMediaCall::SActive:
			Ui_.StatusLabel_->setText (tr ("Active"));
			break;
		case IMediaCall::SDisconnecting:
			Ui_.StatusLabel_->setText (tr ("Disconnecting"));
			break;
		case IMediaCall::SFinished:
			scheduleDelete ();
			break;
		}
	}

	void CallChatWidget::on_AcceptButton__released ()
	{
		Call_->Accept ();
	}

	void CallChatWidget::on_HangupButton__released ()
	{
		Call_->Hangup ();
	}

	void CallChatWidget::scheduleDelete ()
	{
		Ui_.StatusLabel_->setText (tr ("No active call"));
		QTimer::singleShot (3000,
				this,
				SLOT (deleteLater ()));
	}
}
}
