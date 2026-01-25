/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include <QIODevice>

class QAudioFormat;

namespace LC::Azoth::Emitters
{
	class MediaCall;
}

namespace LC::Azoth
{
	class IMediaCall
	{
	protected:
		virtual ~IMediaCall () = default;
	public:
		virtual Emitters::MediaCall& GetMediaCallEmitter () = 0;

		enum Direction
		{
			DIn,
			DOut
		};

		enum State
		{
			SConnecting,
			SActive,
			SDisconnecting,
			SFinished
		};

		virtual Direction GetDirection () const = 0;

		virtual QString GetSourceID () const = 0;

		virtual void Accept () = 0;

		virtual void Hangup () = 0;

		virtual QIODevice* GetAudioDevice () = 0;

		virtual QAudioFormat GetAudioReadFormat () const = 0;

		virtual QAudioFormat GetAudioWriteFormat () const = 0;

		virtual QIODevice* GetVideoDevice () = 0;
	};
}

Q_DECLARE_INTERFACE (LC::Azoth::IMediaCall,
		"org.Deviant.LeechCraft.Azoth.IMediaCall/1.0")
