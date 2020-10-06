/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QString>

class QDataStream;

namespace LC::Azoth::Sarin
{
	struct ToxAccountConfiguration
	{
		bool AllowUDP_ = true;
		bool AllowIPv6_ = true;
		bool AllowLocalDiscovery_ = true;
		bool UdpHolePunching_ = true;

		QString ProxyHost_;
		int ProxyPort_;

		ToxAccountConfiguration () = default;
		ToxAccountConfiguration (bool, bool, const QString&, int);
	};

	bool operator== (const ToxAccountConfiguration&, const ToxAccountConfiguration&);
	bool operator!= (const ToxAccountConfiguration&, const ToxAccountConfiguration&);

	QDataStream& operator<< (QDataStream&, const ToxAccountConfiguration&);
	QDataStream& operator>> (QDataStream&, ToxAccountConfiguration&);
}
