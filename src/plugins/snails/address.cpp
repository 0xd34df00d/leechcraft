/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "address.h"
#include <QDebug>
#include <QDataStream>

namespace LC::Snails
{
	QDataStream& operator<< (QDataStream& out, AddressType a)
	{
		out << static_cast<quint16> (a);
		return out;
	}

	QDataStream& operator>> (QDataStream& in, AddressType& a)
	{
		quint16 t = 0;
		in >> t;
		a = static_cast<AddressType> (t);
		return in;
	}

	QString GetNiceMail (const Address& addr)
	{
		return addr.Name_.isEmpty () ?
				addr.Email_ :
				addr.Name_ + " <" + addr.Email_ + ">";
	}

	QDebug operator<< (QDebug dbg, const Address& addr)
	{
		QDebugStateSaver saver { dbg };
		dbg.nospace () << "Addr { name: `" << addr.Name_
				<< "`, email: `" << addr.Email_
				<< "` }";
		return dbg;
	}

	QDataStream& operator<< (QDataStream& out, const Address& addr)
	{
		out << static_cast<quint8> (1);
		out << addr.Name_
				<< addr.Email_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, Address& addr)
	{
		quint8 version;
		in >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return in;
		}

		in >> addr.Name_
			>> addr.Email_;

		return in;
	}
}
