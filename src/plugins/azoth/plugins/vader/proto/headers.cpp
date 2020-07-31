/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "headers.h"
#include <QtDebug>
#include "exceptions.h"
#include "conversions.h"

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	const quint32 HeaderMagic = 0xDEADBEEF;
	const quint32 ProtoMajor = 1;
	const quint32 ProtoMinor = 22;
	const quint32 ProtoFull = (ProtoMajor << 16) | ProtoMinor;

	Header::Header (QByteArray& outer)
	{
		QByteArray ba = outer;

		FromMRIM (ba, Magic_, Proto_, Seq_, MsgType_, DataLength_, From_, FromPort_);
		if (ba.size () < 16)
			throw TooShortBA ("Too short bytearray to deserialize the header");
		memcpy (Reserved_, ba.constData (), 16);
		ba = ba.mid (16);

		outer = ba;
	}

	Header::Header (quint32 msgType, quint32 seq)
	: Magic_ (HeaderMagic)
	, Proto_ (ProtoFull)
	, Seq_ (seq)
	, MsgType_ (msgType)
	, DataLength_ (0)
	, From_ (0)
	, FromPort_ (0)
	{
		memset (Reserved_, 0, 16);
	}

	QByteArray Header::Serialize () const
	{
		return ToMRIM (Magic_, Proto_, Seq_, MsgType_, DataLength_, From_, FromPort_) + QByteArray (16, 0);
	}
}
}
}
}
