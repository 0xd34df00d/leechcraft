/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "conversions.h"
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

	QByteArray ToMRIM1251 (const QString& string)
	{
		return QTextCodec::codecForName ("Windows-1251")->fromUnicode (string);
	}

	QByteArray ToMRIM16 (const QString& string)
	{
		return QTextCodec::codecForName ("UTF-16LE")->fromUnicode (string);
	}

	QByteArray ToMRIM (const QString& string)
	{
		return ToMRIM (ToMRIM1251 (string));
	}

	QByteArray ToMRIM (const QByteArray& string)
	{
		return ToMRIM (static_cast<quint32> (string.size ())) + string;
	}

	QByteArray ToMRIM (const UIDL& id)
	{
		return id;
	}

	QByteArray ToMRIM (quint32 num)
	{
		QByteArray result (4, 0);
		qToLittleEndian (num, reinterpret_cast<uchar*> (result.data ()));
		return result;
	}

	QByteArray ToMRIM (int i)
	{
		return ToMRIM (static_cast<quint32> (i));
	}

	QByteArray ToMRIM ()
	{
		return QByteArray ();
	}

	UIDL::UIDL ()
	{
	}

	UIDL::UIDL (const UIDL& uidl)
	: ID_ (uidl.ID_)
	{
	}

	UIDL::UIDL (const QByteArray& id)
	: ID_ (id)
	{
	}

	UIDL& UIDL::operator= (const QByteArray& id)
	{
		ID_ = id;
		return *this;
	}

	UIDL::operator QByteArray () const
	{
		return ID_;
	}

	QString FromMRIM1251 (const QByteArray& ba)
	{
		return QTextCodec::codecForName ("Windows-1251")->toUnicode (ba);
	}

	QString FromMRIM16 (const QByteArray& ba)
	{
		return QTextCodec::codecForName ("UTF-16LE")->toUnicode (ba);
	}

	void FromMRIM (QByteArray& lps, EncoderProxy& proxy)
	{
		QByteArray ba;
		FromMRIM (lps, ba);

		proxy = ba;
	}

	void FromMRIM (QByteArray& lps, UIDL& id)
	{
		if (lps.size () < 8)
			throw TooShortBA ("Unable to deserialize UIDL: premature end");

		id = lps.left (8);
		lps = lps.mid (8);
	}

	void FromMRIM (QByteArray& lps, QByteArray& str)
	{
		quint32 size = 0;
		FromMRIM (lps, size);
		if (size > static_cast<quint32> (lps.size ()))
			throw TooShortBA ("Unable to deserialize QString: premature end");

		str = lps.left (size);
		lps = lps.mid (size);
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
