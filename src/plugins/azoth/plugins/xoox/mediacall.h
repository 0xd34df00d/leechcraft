/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QXmppCallManager.h>
#include <interfaces/azoth/imediacall.h>

#ifndef ENABLE_MEDIACALLS
#error Dont include this if media calls are disabled.
#endif

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class GlooxAccount;

	class MediaCall : public QObject
					, public IMediaCall
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMediaCall)

		QXmppCall *Call_;
		GlooxAccount *Account_;
	public:
		MediaCall (GlooxAccount*, QXmppCall*);

		Direction GetDirection () const;
		QString GetSourceID () const;
		void Accept ();
		void Hangup ();
		QIODevice* GetAudioDevice ();
		QAudioFormat GetAudioReadFormat () const;
		QAudioFormat GetAudioWriteFormat () const;
		QIODevice* GetVideoDevice ();
	private slots:
		void handleStateChanged (QXmppCall::State);
	signals:
		void stateChanged (LC::Azoth::IMediaCall::State);
		void audioModeChanged (QIODevice::OpenMode);

		void readFormatChanged ();
		void writeFormatChanged ();
	};
}
}
}
