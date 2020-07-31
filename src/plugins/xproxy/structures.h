/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QNetworkProxy>
#include <util/sll/regexp.h>

class QDataStream;

namespace LC
{
namespace XProxy
{
	struct ReqTarget
	{
		Util::RegExp Host_;
		int Port_;
		QStringList Protocols_;
	};
	struct Proxy
	{
		QNetworkProxy::ProxyType Type_;
		QString Host_;
		int Port_;
		QString User_;
		QString Pass_;

		operator QNetworkProxy () const;
	};

	bool operator< (const Proxy&, const Proxy&);
	bool operator== (const Proxy&, const Proxy&);

	using Entry_t = QPair<ReqTarget, Proxy>;

	QDataStream& operator<< (QDataStream&, const Proxy&);
	QDataStream& operator>> (QDataStream&, Proxy&);
	QDataStream& operator<< (QDataStream&, const ReqTarget&);
	QDataStream& operator>> (QDataStream&, ReqTarget&);
}
}

Q_DECLARE_METATYPE (QList<LC::XProxy::Entry_t>)
