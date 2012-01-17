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

#include "mediacall.h"
#include <QAudioFormat>
#include <QXmppCallManager.h>
#include <QXmppRtpChannel.h>
#include "clientconnection.h"
#include "glooxaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	MediaCall::MediaCall (GlooxAccount *acc, QXmppCall *call)
	: QObject (call)
	, Call_ (call)
	, Account_ (acc)
	{
		connect (Call_,
				SIGNAL (stateChanged (QXmppCall::State)),
				this,
				SLOT (handleStateChanged (QXmppCall::State)));
	}
	
	IMediaCall::Direction MediaCall::GetDirection () const
	{
		switch (Call_->direction ())
		{
		case QXmppCall::IncomingDirection:
			return DIn;
		case QXmppCall::OutgoingDirection:
			return DOut;
		}
	}
	
	QString MediaCall::GetSourceID () const
	{
		QString jid;
		QString var;
		ClientConnection::Split (Call_->jid (), &jid, &var);
		return Account_->GetAccountID () + '_' + jid;
	}
	
	void MediaCall::Accept ()
	{
		Call_->accept ();
	}
	
	void MediaCall::Hangup ()
	{
		Call_->hangup ();
	}
	
	QIODevice* MediaCall::GetAudioDevice ()
	{
		QXmppJinglePayloadType payload = Call_->audioChannel ()->payloadType ();
		qDebug () << "INFO" << payload.name () << payload.parameters ();
		qDebug () << payload.channels () << payload.clockrate ();
		return Call_->audioChannel ();
	}
	
	QAudioFormat MediaCall::GetAudioFormat ()
	{
		const QXmppJinglePayloadType& payload =
				Call_->audioChannel ()->payloadType ();
		QAudioFormat result;
#if QT_VERSION >= 0x040700
		result.setSampleRate (payload.clockrate ());
		result.setChannelCount (payload.channels ());
#endif
		result.setSampleSize (16);
		result.setCodec ("audio/pcm");
		result.setByteOrder (QAudioFormat::LittleEndian);
		result.setSampleType (QAudioFormat::SignedInt);
		return result;
	}
	
	QIODevice* MediaCall::GetVideoDevice ()
	{
		return 0;
	}
	
	void MediaCall::handleStateChanged (QXmppCall::State state)
	{
		emit stateChanged (static_cast<State> (state));
	}
}
}
}