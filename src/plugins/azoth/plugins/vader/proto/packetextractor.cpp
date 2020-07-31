/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "packetextractor.h"
#include <QtDebug>
#include "exceptions.h"
#include "headers.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	bool PacketExtractor::MayGetPacket () const
	{
#ifdef PROTOCOL_LOGGING
		qDebug () << Q_FUNC_INFO;
#endif
		if (Buffer_.isEmpty ())
			return false;

		try
		{
			QByteArray tmp { Buffer_ };
			Header h { tmp };
#ifdef PROTOCOL_LOGGING
			qDebug () << h.DataLength_ << tmp.size ();
#endif
			if (h.DataLength_ > static_cast<quint32> (tmp.size ()))
				return false;
		}
		catch (const TooShortBA&)
		{
			qDebug () << "too short bytearray";
			return false;
		}

#ifdef PROTOCOL_LOGGING
		qDebug () << "may get packet";
#endif

		return true;
	}

	HalfPacket PacketExtractor::GetPacket ()
	{
		Header h (Buffer_);
		const QByteArray& data = Buffer_.left (h.DataLength_);
		if (h.DataLength_)
			Buffer_ = Buffer_.mid (h.DataLength_);
		return { h, data };
	}

	void PacketExtractor::Clear ()
	{
		Buffer_.clear ();
	}

	PacketExtractor& PacketExtractor::operator+= (const QByteArray& ba)
	{
		Buffer_ += ba;
		return *this;
	}
}
}
}
}
