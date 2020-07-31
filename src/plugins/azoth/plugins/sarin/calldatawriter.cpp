/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "calldatawriter.h"
#include <util/threads/futures.h>
#include <util/sll/visitor.h>
#include "callmanager.h"

namespace LC
{
namespace Azoth
{
namespace Sarin
{
	CallDataWriter::CallDataWriter (int32_t callIdx, CallManager *callMgr, QObject *parent)
	: QObject { parent }
	, CallIdx_ { callIdx}
	, Mgr_ { callMgr }
	{
	}

	qint64 CallDataWriter::WriteData (const QAudioFormat& fmt, const QByteArray& data)
	{
		if (IsWriting_)
		{
			qDebug () << Q_FUNC_INFO << "already writing, queuing up" << data.size ();
			Buffer_ += data;
			return data.size ();
		}

		qDebug () << Q_FUNC_INFO << "sending" << data.size () << "with buffer of size" << Buffer_.size ();

		IsWriting_ = true;

		Util::Sequence (this, Mgr_->WriteData (CallIdx_, fmt, Buffer_ + data)) >>
				Util::Visitor
				{
					[this] (const QByteArray& leftover) { Buffer_.prepend (leftover); },
					[] (const auto& err)
					{
						qWarning () << Q_FUNC_INFO
								<< "error writing frame:"
								<< Util::Visit (err, [] (auto&& e) { return e.what (); });
					}
				}.Finally ([this] { IsWriting_ = false; });

		Buffer_.clear ();

		return data.size ();
	}
}
}
}
