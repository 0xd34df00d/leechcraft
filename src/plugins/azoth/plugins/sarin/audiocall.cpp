/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "audiocall.h"
#include <QFuture>
#include <util/sll/functional.h>
#include <util/sll/slotclosure.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include "threadexceptions.h"
#include "audiocalldevice.h"
#include "toxcontact.h"

namespace LC::Azoth::Sarin
{
	namespace
	{
		void InitFormat (QAudioFormat& format)
		{
			format.setSampleSize (16);
			format.setByteOrder (QSysInfo::ByteOrder == QSysInfo::BigEndian ?
					QAudioFormat::BigEndian :
					QAudioFormat::LittleEndian);
			format.setCodec ("audio/pcm");
			format.setSampleType (QAudioFormat::SignedInt);
		}
	}

	AudioCall::AudioCall (const ToxContact *contact, CallManager *callMgr, Direction dir)
	: SourceId_ { contact->GetEntryID () }
	, SourcePubkey_ { contact->GetPubKey () }
	, Dir_ { dir }
	, CallMgr_ { callMgr }
	{
		connect (CallMgr_,
				&CallManager::callStateChanged,
				this,
				&AudioCall::HandleCallStateChanged);
		connect (CallMgr_,
				&CallManager::gotFrameParams,
				this,
				&AudioCall::HandleReadFrameParams);

		InitFormat (ReadFmt_);

		InitFormat (WriteFmt_);
		WriteFmt_.setChannelCount (1);
		WriteFmt_.setSampleRate (24000);
	}

	void AudioCall::SetCallIdx (const std::optional<qint32>& idx)
	{
		if (!idx)
		{
			qWarning () << Q_FUNC_INFO
					<< "no call idx";
			emit stateChanged (SFinished);
			return;
		}

		CallIdx_ = *idx;

		Device_ = std::make_shared<AudioCallDevice> (CallIdx_, CallMgr_);
		Device_->open (QIODevice::ReadWrite);
		Device_->SetWriteFormat (WriteFmt_);

		if (Dir_ == DOut)
			InitiateCall ();
	}

	IMediaCall::Direction AudioCall::GetDirection () const
	{
		return Dir_;
	}

	QString AudioCall::GetSourceID () const
	{
		return SourceId_;
	}

	void AudioCall::Accept ()
	{
		if (Dir_ == DOut)
			return;

		Util::Sequence (this, CallMgr_->AcceptCall (CallIdx_)) >>
				Util::Visitor
				{
					Util::BindMemFn (&AudioCall::HandleWriteParams, this),
					[this] (const auto& err)
					{
						qWarning () << Q_FUNC_INFO
								<< "error accepting the call:"
								<< Util::Visit (err, [] (auto&& e) { return e.what (); });
						emit stateChanged (SFinished);
					}
				};
	}

	void AudioCall::Hangup ()
	{
	}

	QIODevice* AudioCall::GetAudioDevice ()
	{
		return Device_.get ();
	}

	QAudioFormat AudioCall::GetAudioReadFormat () const
	{
		return ReadFmt_;
	}

	QAudioFormat AudioCall::GetAudioWriteFormat () const
	{
		return WriteFmt_;
	}

	QIODevice* AudioCall::GetVideoDevice ()
	{
		return nullptr;
	}

	void AudioCall::InitiateCall ()
	{
		Util::Sequence (this, CallMgr_->InitiateCall (SourcePubkey_.toUtf8 ())) >>
				Util::Visitor
				{
					[this] (const CallManager::AudioFormatParams& params)
					{
						HandleWriteParams (params);
						emit stateChanged (SConnecting);
					},
					[this] (const auto& err)
					{
						qWarning () << Q_FUNC_INFO
								<< "error initiating the call:"
								<< Util::Visit (err, [] (auto&& e) { return e.what (); });
						emit stateChanged (SFinished);
					}
				};
	}

	void AudioCall::HandleWriteParams (const CallManager::AudioFormatParams&)
	{
	}

	void AudioCall::HandleReadFrameParams (int32_t callIdx, int channels, int sampleRate)
	{
		if (callIdx != CallIdx_)
			return;

		if (ReadFmt_.channelCount () != channels ||
				ReadFmt_.sampleRate () != sampleRate)
		{
			ReadFmt_.setChannelCount (channels);
			ReadFmt_.setSampleRate (sampleRate);

			emit readFormatChanged ();
		}
	}

	void AudioCall::HandleCallStateChanged (int32_t callIdx, uint32_t state)
	{
		if (callIdx != CallIdx_)
			return;

		qDebug () << Q_FUNC_INFO << state;

		if (state & TOXAV_FRIEND_CALL_STATE_ERROR)
		{
			emit stateChanged (SFinished);
			qWarning () << Q_FUNC_INFO
					<< "got error state";
			return;
		}

		if (state & TOXAV_FRIEND_CALL_STATE_FINISHED)
		{
			emit stateChanged (SFinished);
			return;
		}

		if ((state & TOXAV_FRIEND_CALL_STATE_ACCEPTING_V) ||
				(state & TOXAV_FRIEND_CALL_STATE_SENDING_V))
			qWarning () << Q_FUNC_INFO
					<< "cannot handle videos yet";

		if (state & TOXAV_FRIEND_CALL_STATE_ACCEPTING_A)
			CurrentMode_ |= QIODevice::ReadOnly;

		if (state & TOXAV_FRIEND_CALL_STATE_SENDING_A)
			CurrentMode_ |= QIODevice::WriteOnly;

		if (CurrentMode_ == QIODevice::ReadWrite)
			emit stateChanged (SActive);

		emit audioModeChanged (CurrentMode_);
	}
}
