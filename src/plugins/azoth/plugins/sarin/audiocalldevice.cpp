/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "audiocalldevice.h"
#include <cstring>
#include <QtDebug>
#include "callmanager.h"

namespace LC::Azoth::Sarin
{
	AudioCallDevice::AudioCallDevice (int32_t callIdx, CallManager *manager)
	: Idx_ { callIdx }
	, Manager_ { manager }
	, DataWriter_ { std::make_unique<CallDataWriter> (callIdx, manager) }
	{
		connect (Manager_,
				&CallManager::gotFrame,
				this,
				&AudioCallDevice::HandleGotFrame);
		connect (DataWriter_.get (),
				&CallDataWriter::gotError,
				this,
				&AudioCallDevice::HandleWriteError);
	}

	void AudioCallDevice::SetWriteFormat (const QAudioFormat& fmt)
	{
		WriteFmt_ = fmt;
	}

	qint64 AudioCallDevice::bytesAvailable () const
	{
		return ReadBuffer_.size () + QIODevice::bytesAvailable ();
	}

	bool AudioCallDevice::isSequential () const
	{
		return true;
	}

	qint64 AudioCallDevice::readData (char *data, qint64 maxlen)
	{
		const auto len = std::min<qint64> (maxlen, ReadBuffer_.size ());

		std::memcpy (data, ReadBuffer_.constData (), len);
		ReadBuffer_.remove (0, len);
		return len;
	}

	qint64 AudioCallDevice::writeData (const char *data, qint64 len)
	{
		if (!WriteFmt_)
		{
			qWarning () << Q_FUNC_INFO
					<< "no write format has been set for call"
					<< Idx_;

			setErrorString ("AudioCallDevice::writeData(): no write QAudioFormat has been set.");

			return -1;
		}

		return DataWriter_->WriteData (*WriteFmt_, { data, static_cast<int> (len) });
	}

	void AudioCallDevice::HandleGotFrame (int32_t callIdx, const QByteArray& data)
	{
		if (callIdx != Idx_)
			return;

		ReadBuffer_ += data;
		qDebug () << Q_FUNC_INFO
				<< "got frame of size" << data.size ()
				<< "; total size:" << ReadBuffer_.size ();
		emit readyRead ();
	}

	void AudioCallDevice::HandleWriteError (const QString& errStr)
	{
		setErrorString (errStr);
	}
}
