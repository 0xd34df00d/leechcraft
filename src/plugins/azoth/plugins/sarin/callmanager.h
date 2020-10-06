/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <variant>
#include <QObject>
#include <tox/toxav.h>
#include <util/sll/either.h>
#include "threadexceptions.h"

template<typename T>
class QFuture;

class QAudioFormat;

namespace LC::Azoth::Sarin
{
	class ToxThread;

	template<typename... Errors>
	using Error_t = std::variant<Errors...>;

	class CallManager : public QObject
	{
		Q_OBJECT

		ToxThread * const Thread_;
		std::unique_ptr<ToxAV, decltype (&toxav_kill)> ToxAv_;

		struct ToxAvThread;
		std::shared_ptr<ToxAvThread> ToxAvThread_;
	public:
		CallManager (ToxThread*, Tox*, QObject* = nullptr);

		struct AudioFormatParams
		{
			int AudioBitrate_;
		};

		template<typename... Errors>
		using CallStartResult = Util::Either<Error_t<Errors...>, AudioFormatParams>;

		using InitiateResult = CallStartResult<UnknownFriendException, CallInitiateException>;
		QFuture<InitiateResult> InitiateCall (const QByteArray& pkey);

		using AcceptCallResult = CallStartResult<CallAnswerException>;
		QFuture<AcceptCallResult> AcceptCall (int32_t callIdx);

		using WriteResult = Util::Either<Error_t<FrameSendException>, QByteArray>;
		QFuture<WriteResult> WriteData (int32_t callIdx, const QAudioFormat&, QByteArray data);
	private:
		void HandleIncomingCall (int32_t callIdx);
		void HandleStateChanged (int32_t friendIdx, uint32_t state);

		void HandleAudio (int32_t call, const int16_t *frames, int size, int channels, int rate);
	signals:
		void gotIncomingCall (const QByteArray& pubkey, int32_t callIdx);
		void callStateChanged (int32_t callidx, uint32_t state);

		void gotFrameParams (int32_t call, int channels, int rate);
		void gotFrame (int32_t call, const QByteArray&);
	};
}
