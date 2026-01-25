/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "callchatwidget.h"
#include <QTimer>
#include <util/azoth/emitters/mediacall.h>

namespace LC::Azoth
{
	CallChatWidget::CallChatWidget (QObject *callObj, QWidget *parent)
	: QWidget { parent }
	{
		Ui_.setupUi (this);

		Ui_.StatusLabel_->setText (tr ("Initializing..."));

		const auto call = qobject_cast<IMediaCall*> (callObj);

		if (call->GetDirection () == IMediaCall::DOut)
			Ui_.AcceptButton_->hide ();

		connect (Ui_.AcceptButton_,
				&QPushButton::released,
				callObj,
				[call] { call->Accept (); });
		connect (Ui_.HangupButton_,
				&QPushButton::released,
				callObj,
				[call] { call->Hangup (); });

		const auto handleFinished = [this]
		{
			Ui_.StatusLabel_->setText (tr ("No active call"));

			using namespace std::chrono_literals;
			QTimer::singleShot (3s,
					this,
					&QObject::deleteLater);
		};

		connect (callObj,
				&QObject::destroyed,
				this,
				handleFinished);
		connect (&call->GetMediaCallEmitter (),
				&Emitters::MediaCall::stateChanged,
				this,
				[=, this] (IMediaCall::State state)
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
						handleFinished ();
						break;
					}
				});
	}
}
