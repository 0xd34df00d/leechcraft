/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QIODevice>
#include <QAudioFormat>
#include "calldatawriter.h"

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	class CallManager;

	class AudioCallDevice : public QIODevice
	{
		const int32_t Idx_;
		CallManager * const Manager_;

		QByteArray ReadBuffer_;

		std::unique_ptr<CallDataWriter> DataWriter_;

		std::optional<QAudioFormat> WriteFmt_;
	public:
		AudioCallDevice (int32_t, CallManager*);

		void SetWriteFormat (const QAudioFormat&);

		qint64 bytesAvailable () const override;
		bool isSequential () const override;
	protected:
		qint64 readData (char *data, qint64 maxlen) override;
		qint64 writeData (const char *data, qint64 len) override;
	private:
		void HandleGotFrame (int32_t, const QByteArray&);
		void HandleWriteError (const QString&);
	};
}
}
}
