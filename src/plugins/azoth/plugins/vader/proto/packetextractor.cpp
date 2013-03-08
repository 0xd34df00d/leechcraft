/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "packetextractor.h"
#include <QtDebug>
#include "exceptions.h"
#include "headers.h"

namespace LeechCraft
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
			QByteArray tmp (Buffer_);
			Header h (tmp);
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
