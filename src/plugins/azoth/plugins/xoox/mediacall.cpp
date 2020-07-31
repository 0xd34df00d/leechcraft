/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "mediacall.h"
#include <QAudioFormat>
#include <QtDebug>
#include <QXmppCallManager.h>
#include <QXmppRtpChannel.h>
#include "clientconnection.h"
#include "glooxaccount.h"

namespace LC
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
		connect (Call_,
				SIGNAL (audioModeChanged (QIODevice::OpenMode)),
				this,
				SIGNAL (audioModeChanged (QIODevice::OpenMode)));
	}

	IMediaCall::Direction MediaCall::GetDirection () const
	{
		switch (Call_->direction ())
		{
		case QXmppCall::IncomingDirection:
			return DIn;
		case QXmppCall::OutgoingDirection:
			return DOut;
		default:
			qWarning () << Q_FUNC_INFO
					<< "unknown direction"
					<< Call_->direction ();
			return DIn;
		}
	}

	QString MediaCall::GetSourceID () const
	{
		return Account_->GetAccountID () + '_' + ClientConnection::Split (Call_->jid ()).Bare_;
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
		const auto& payload = Call_->audioChannel ()->payloadType ();
		qDebug () << "INFO" << payload.name () << payload.parameters ();
		qDebug () << payload.channels () << payload.clockrate ();
		return Call_->audioChannel ();
	}

	QAudioFormat MediaCall::GetAudioReadFormat () const
	{
		const auto& payload = Call_->audioChannel ()->payloadType ();
		QAudioFormat result;
		result.setSampleRate (payload.clockrate ());
		result.setChannelCount (payload.channels ());
		result.setSampleSize (16);
		result.setCodec ("audio/pcm");
		result.setByteOrder (QAudioFormat::LittleEndian);
		result.setSampleType (QAudioFormat::SignedInt);
		return result;
	}

	QAudioFormat MediaCall::GetAudioWriteFormat () const
	{
		return GetAudioReadFormat ();
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
