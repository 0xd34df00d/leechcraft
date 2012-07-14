/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "callchatwidget.h"
#include <QTimer>

namespace LeechCraft
{
namespace Azoth
{
	CallChatWidget::CallChatWidget (QObject *call, QWidget *parent)
	: QWidget (parent)
	, CallObject_ (call)
	, Call_ (qobject_cast<IMediaCall*> (call))
	{
		Ui_.setupUi (this);
		
		Ui_.StatusLabel_->setText (tr ("Initializing..."));
		
		connect (call,
				SIGNAL (destroyed ()),
				this,
				SLOT (scheduleDelete ()));
		
		connect (call,
				SIGNAL (stateChanged (LeechCraft::Azoth::IMediaCall::State)),
				this,
				SLOT (handleStateChanged (LeechCraft::Azoth::IMediaCall::State)));
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
