/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "structures.h"
#include <QDataStream>

namespace LC
{
namespace XProxy
{
	Proxy::operator QNetworkProxy () const
	{
		return { Type_, Host_, static_cast<quint16> (Port_), User_, Pass_ };
	}

	namespace
	{
		template<typename Class, typename G, typename... Gs>
		struct IsLessImpl
		{
			bool operator() (const Class& left, const Class& right, G g, Gs... gs) const
			{
				if (left.*g != right.*g)
					return left.*g < right.*g;

				return IsLessImpl<Class, Gs...> {} (left, right, gs...);
			}
		};

		template<typename Class, typename G>
		struct IsLessImpl<Class, G>
		{
			bool operator() (const Class& left, const Class& right, G g) const
			{
				if (left.*g != right.*g)
					return left.*g < right.*g;

				return false;
			}
		};

		template<typename Class, typename... Gs>
		bool IsLess (const Class& left, const Class& right, Gs... gs)
		{
			return IsLessImpl<Class, Gs...> {} (left, right, gs...);
		}
	}

	bool operator< (const Proxy& left, const Proxy& right)
	{
		return IsLess (left, right, &Proxy::Type_, &Proxy::Port_, &Proxy::Host_, &Proxy::User_, &Proxy::Pass_);
	}

	bool operator== (const Proxy& left, const Proxy& right)
	{
		return !(left < right) && !(right < left);
	}

	QDataStream& operator<< (QDataStream& out, const Proxy& p)
	{
		out << static_cast<quint8> (1);
		out << static_cast<qint8> (p.Type_)
			<< p.Host_
			<< p.Port_
			<< p.User_
			<< p.Pass_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, Proxy& p)
	{
		quint8 ver = 0;
		in >> ver;
		if (ver != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return in;
		}

		qint8 type = 0;
		in >> type
			>> p.Host_
			>> p.Port_
			>> p.User_
			>> p.Pass_;
		p.Type_ = static_cast<QNetworkProxy::ProxyType> (type);

		return in;
	}

	QDataStream& operator<< (QDataStream& out, const ReqTarget& t)
	{
		out << static_cast<quint8> (2);
		out << t.Host_
			<< t.Port_
			<< t.Protocols_;
		return out;
	}

	QDataStream& operator>> (QDataStream& in, ReqTarget& t)
	{
		quint8 ver = 0;
		in >> ver;
		if (ver < 1 || ver > 2)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version"
					<< ver;
			return in;
		}

		if (ver == 1)
		{
			QRegExp rx;
			in >> rx;
			t.Host_ = Util::RegExp { rx.pattern (), rx.caseSensitivity () };
		}
		else
			in >> t.Host_;

		in >> t.Port_
			>> t.Protocols_;
		return in;
	}
}
}
