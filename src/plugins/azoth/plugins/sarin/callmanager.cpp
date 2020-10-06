/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "callmanager.h"
#include <QFuture>
#include <QAudioFormat>
#include <QElapsedTimer>
#include <util/threads/futures.h>
#include "toxthread.h"
#include "util.h"
#include "threadexceptions.h"

namespace LC::Azoth::Sarin
{
	struct CallManager::ToxAvThread : public QThread
	{
		std::atomic_flag ShouldStop_ = ATOMIC_FLAG_INIT;

		ToxAV * const ToxAv_;
	public:
		ToxAvThread (ToxAV *toxAv)
		: ToxAv_ { toxAv }
		{
		}

		~ToxAvThread ()
		{
			Stop ();
			wait (2000);
		}

		void Stop ()
		{
			ShouldStop_.clear (std::memory_order_relaxed);
		}
	protected:
		void run () override
		{
			ShouldStop_.test_and_set (std::memory_order_relaxed);

			while (ShouldStop_.test_and_set (std::memory_order_relaxed))
			{
				const auto delay = toxav_iteration_interval (ToxAv_);
				msleep (delay);
				toxav_iterate (ToxAv_);
			}
		}
	};

	CallManager::CallManager (ToxThread *thread, Tox *tox, QObject *parent)
	: QObject { parent }
	, Thread_ { thread }
	, ToxAv_
	{
		[tox]
		{
			TOXAV_ERR_NEW error;
			const auto toxAV = toxav_new (tox, &error);
			if (error != TOXAV_ERR_NEW_OK)
				throw std::runtime_error { "Failed to create tox AV instance: " + std::to_string (error) };
			return toxAV;
		} (),
		&toxav_kill
	}
	, ToxAvThread_ { std::make_shared<ToxAvThread> (ToxAv_.get ()) }
	{
		toxav_callback_call (ToxAv_.get (),
				[] (ToxAV*, uint32_t friendNum, bool audio, bool video, void *udata)
				{
					static_cast<CallManager*> (udata)->HandleIncomingCall (friendNum);
				},
				this);
		toxav_callback_call_state (ToxAv_.get (),
				[] (ToxAV*, uint32_t friendNum, uint32_t state, void *udata)
				{
					static_cast<CallManager*> (udata)->HandleStateChanged (friendNum, state);
				},
				this);
		toxav_callback_audio_receive_frame (ToxAv_.get (),
				[] (ToxAV*, uint32_t friendNum, const int16_t *frames,
						size_t size, uint8_t channels, uint32_t rate,
						void *udata)
				{
					static_cast<CallManager*> (udata)->HandleAudio (friendNum, frames, size, channels, rate);
				},
				this);

		ToxAvThread_->start (QThread::IdlePriority);
	}

	namespace
	{
		const int AudioBitRate = 16;
		const int VideoBitRate = 0;
	}

	QFuture<CallManager::InitiateResult> CallManager::InitiateCall (const QByteArray& pkey)
	{
		return Thread_->ScheduleFunction ([this, pkey] (Tox *tox)
				{
					return InitiateResult::FromMaybe (GetFriendId (tox, pkey), UnknownFriendException {}) >>
							[this] (qint32 id)
							{
								TOXAV_ERR_CALL error;
								toxav_call (ToxAv_.get (), id, AudioBitRate, VideoBitRate, &error);
								return error != TOXAV_ERR_CALL_OK ?
										InitiateResult::Left (CallInitiateException { error }) :
										InitiateResult::Right ({ AudioBitRate });
							};
				});
	}

	QFuture<CallManager::AcceptCallResult> CallManager::AcceptCall (int32_t friendIdx)
	{
		return Thread_->ScheduleFunction ([this, friendIdx] (Tox*)
				{
					TOXAV_ERR_ANSWER error;
					toxav_answer (ToxAv_.get (), friendIdx, AudioBitRate, VideoBitRate, &error);
					return error != TOXAV_ERR_ANSWER_OK ?
							AcceptCallResult::Left (CallAnswerException { error }) :
							AcceptCallResult::Right ({ AudioBitRate });
				});
	}

	QFuture<CallManager::WriteResult> CallManager::WriteData (int32_t callIdx,
			const QAudioFormat& fmt, QByteArray data)
	{
		return Thread_->ScheduleFunction ([=] (Tox*) mutable
				{
					qDebug () << Q_FUNC_INFO;
					while (true)
					{
						const auto totalSamples = data.size () / sizeof (int16_t) / fmt.channelCount ();

						const float allowedLengths [] = { 2.5, 5, 10, 20, 40, 60 };

						/* Tox docs say that valid samples count is subject to
						* (samples count) = (sample rate) * (audio length) / 1000
						* thus max audio length is
						* audio length = 1000 * (samples count) / (sample rate)
						*/
						const auto maxAudioLength = 1000.0 * totalSamples / fmt.sampleRate ();
						if (maxAudioLength < allowedLengths [0])
							return WriteResult::Right (data);

						const auto allowedLengthPos = std::upper_bound (std::begin (allowedLengths),
								std::end (allowedLengths), maxAudioLength);
						const auto allowedLength = *std::prev (allowedLengthPos);

						const auto samplesToSend = fmt.sampleRate () * allowedLength / 1000;

						qDebug () << "gonna send"
								<< allowedLength << "of" << maxAudioLength << "ms or"
								<< samplesToSend << "of" << totalSamples << "samples";

						TOXAV_ERR_SEND_FRAME error;
						toxav_audio_send_frame (ToxAv_.get (),
								callIdx,
								reinterpret_cast<const int16_t*> (data.constData ()),
								samplesToSend,
								fmt.channelCount (),
								fmt.sampleRate (),
								&error);

						if (error != TOXAV_ERR_SEND_FRAME_OK)
							return WriteResult::Left (FrameSendException { error });

						data = data.mid (samplesToSend * sizeof (int16_t) * fmt.channelCount ());
					}
				});
	}

	void CallManager::HandleIncomingCall (int32_t friendNum)
	{
		Util::Sequence (this, Thread_->GetFriendPubkey (friendNum)) >>
				[this, friendNum] (const QByteArray& pubkey) { gotIncomingCall (pubkey, friendNum); };
	}

	void CallManager::HandleStateChanged (int32_t friendIdx, uint32_t state)
	{
		emit callStateChanged (friendIdx, state);
	}

	void CallManager::HandleAudio (int32_t call, const int16_t *frames, int size, int channels, int rate)
	{
		emit gotFrameParams (call, channels, rate);

		const QByteArray data { reinterpret_cast<const char*> (frames), static_cast<int> (size * sizeof (int16_t)) };
		emit gotFrame (call, data);
	}
}
