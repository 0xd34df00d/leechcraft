/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "toxaccountconfiguration.h"
#include <QDataStream>
#include <QtDebug>

namespace LC::Azoth::Sarin
{
	ToxAccountConfiguration::ToxAccountConfiguration (bool udp, bool ipv6, const QString& host, int port)
	: AllowUDP_ { udp }
	, AllowIPv6_ { ipv6 }
	, ProxyHost_ { host }
	, ProxyPort_ { port }
	{
	}

	bool operator== (const ToxAccountConfiguration& c1, const ToxAccountConfiguration& c2)
	{
		return c1.AllowUDP_ == c2.AllowUDP_ &&
				c1.AllowIPv6_ == c2.AllowIPv6_ &&
				c1.AllowLocalDiscovery_ == c2.AllowLocalDiscovery_ &&
				c1.UdpHolePunching_ == c2.UdpHolePunching_ &&
				c1.ProxyPort_ == c2.ProxyPort_ &&
				c1.ProxyHost_ == c2.ProxyHost_;
	}

	bool operator!= (const ToxAccountConfiguration& c1, const ToxAccountConfiguration& c2)
	{
		return !(c1 == c2);
	}

	QDataStream& operator<< (QDataStream& out, const ToxAccountConfiguration& config)
	{
		out << static_cast<quint8> (2)
				<< config.AllowUDP_
				<< config.AllowIPv6_
				<< config.ProxyHost_
				<< config.ProxyPort_
				<< config.AllowLocalDiscovery_
				<< config.UdpHolePunching_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, ToxAccountConfiguration& config)
	{
		quint8 version = 0;
		in >> version;
		if (version < 1 || version > 2)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< version;
			return in;
		}

		in >> config.AllowUDP_
				>> config.AllowIPv6_
				>> config.ProxyHost_
				>> config.ProxyPort_;
		if (version >= 2)
			in >> config.AllowLocalDiscovery_
					>> config.UdpHolePunching_;
		return in;
	}
}
