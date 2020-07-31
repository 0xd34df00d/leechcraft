/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>
#include <QList>
#include <QPair>

namespace LC::Snails
{
	enum class AddressType
	{
		To,
		Cc,
		Bcc,
		From,
		ReplyTo
	};

	inline uint qHash (AddressType ty)
	{
		return static_cast<uint> (ty);
	}

	QDataStream& operator<< (QDataStream&, AddressType);
	QDataStream& operator>> (QDataStream&, AddressType&);

	struct Address
	{
		QString Name_;
		QString Email_;
	};
	using Addresses_t = QList<Address>;

	QDebug operator<< (QDebug, const Address&);

	QDataStream& operator<< (QDataStream&, const Address&);
	QDataStream& operator>> (QDataStream&, Address&);

	QString GetNiceMail (const Address&);
}
