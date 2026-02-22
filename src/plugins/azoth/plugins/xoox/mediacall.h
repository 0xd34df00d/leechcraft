/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/azoth/imediacall.h>
#include <util/azoth/emitters/mediacall.h>

class QAudioFormat;
class QXmppCall;

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
		Emitters::MediaCall Emitter_;
	public:
		MediaCall (GlooxAccount*, QXmppCall*);

		Emitters::MediaCall& GetMediaCallEmitter ();
		Direction GetDirection () const;
		QString GetSourceID () const;
		void Accept ();
		void Hangup ();
		QIODevice* GetAudioDevice ();
		QAudioFormat GetAudioReadFormat () const;
		QAudioFormat GetAudioWriteFormat () const;
		QIODevice* GetVideoDevice ();
	};
}
}
}
