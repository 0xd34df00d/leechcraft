/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>

#ifdef ENABLE_MEDIACALLS
#include <QAudioFormat>
#endif

#include <interfaces/azoth/imediacall.h>
#include "callmanager.h"

template<typename T>
class QFuture;

#ifndef ENABLE_MEDIACALLS
class QAudioFormat;
#endif

namespace LC::Azoth::Sarin
{
	class AudioCallDevice;
	class ToxContact;

	class AudioCall : public QObject
					, public IMediaCall
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::IMediaCall)

		const QString SourceId_;
		const QString SourcePubkey_;
		const Direction Dir_;

		CallManager * const CallMgr_;

		int32_t CallIdx_ = -1;

		QAudioFormat ReadFmt_;
		QAudioFormat WriteFmt_;

		std::shared_ptr<AudioCallDevice> Device_;
		QIODevice::OpenMode CurrentMode_ = QIODevice::NotOpen;
	public:
		AudioCall (const ToxContact*, CallManager*, Direction);

		void SetCallIdx (const std::optional<qint32>&);

		Direction GetDirection () const;
		QString GetSourceID () const;
		void Accept ();
		void Hangup ();
		QIODevice* GetAudioDevice ();

		QAudioFormat GetAudioReadFormat () const;
		QAudioFormat GetAudioWriteFormat () const;

		QIODevice* GetVideoDevice ();
	private:
		void InitiateCall ();
		void HandleWriteParams (const CallManager::AudioFormatParams&);

		void HandleReadFrameParams (int32_t, int, int);
		void HandleCallStateChanged (int32_t, uint32_t);
	signals:
		void stateChanged (LC::Azoth::IMediaCall::State);
		void audioModeChanged (QIODevice::OpenMode);

		void readFormatChanged ();
		void writeFormatChanged ();
	};
}
