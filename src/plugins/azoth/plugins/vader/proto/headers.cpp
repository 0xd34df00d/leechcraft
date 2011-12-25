/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "headers.h"
#include <QTextCodec>
#include <QtEndian>
#include "exceptions.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	const quint32 HeaderMagic = 0xDEADBEEF;
	const quint32 ProtoMajor = 1;
	const quint32 ProtoMinor = 7;
	const quint32 ProtoFull = (ProtoMajor << 16) | ProtoMinor;

	Header::Header (QByteArray& outer)
	{
		QByteArray ba = outer;

		FromMRIM (ba, Magic_, Proto_, Seq_, MsgType_, DataLength_, From_, FromPort_);
		if (ba.size () < 16)
			throw std::runtime_error ("Too short bytearray to deserialize the header");
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

	QByteArray ToMRIM (const QString& string)
	{
		return ToMRIM (string.size ()) + QTextCodec::codecForName ("cp-1251")->fromUnicode (string);
	}

	QByteArray ToMRIM (quint32 num)
	{
		QByteArray result (4, 0);
		qToLittleEndian (num, reinterpret_cast<uchar*> (result.data ()));
		return result;
	}

	QByteArray ToMRIM ()
	{
		return QByteArray ();
	}

	void FromMRIM (QByteArray& lps, QString& str)
	{
		quint32 size = 0;
		FromMRIM (lps, size);
		if (size > static_cast<quint32> (lps.size ()))
			throw TooShortBA ("Unable to deserialize QString: premature end");

		const QByteArray& toDecode = lps.left (size);
		lps = lps.mid (size);

		str = QTextCodec::codecForName ("cp-1251")->toUnicode (toDecode.constData (), size);
	}

	void FromMRIM (QByteArray& ba, quint32& res)
	{
		if (ba.size () < 4)
			throw TooShortBA ("Unable to deserialize quint32: premature end");

		const QByteArray& toDecode = ba.left (4);
		ba = ba.mid (4);
		res = qFromLittleEndian<quint32> (reinterpret_cast<const uchar*> (toDecode.constData ()));
	}

	void FromMRIM (QByteArray&)
	{
	}
}
}
}
}
