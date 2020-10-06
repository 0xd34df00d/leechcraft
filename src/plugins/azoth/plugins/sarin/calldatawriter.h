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
#include "callmanager.h"

class QAudioFormat;

namespace LC::Azoth::Sarin
{
	class CallManager;

	class CallDataWriter : public QObject
	{
		Q_OBJECT

		const int32_t CallIdx_;
		CallManager * const Mgr_;

		QByteArray Buffer_;

		bool IsWriting_ = false;
	public:
		CallDataWriter (int32_t, CallManager*, QObject* = nullptr);

		qint64 WriteData (const QAudioFormat&, const QByteArray&);
	signals:
		void gotError (const QString&);
	};
}
