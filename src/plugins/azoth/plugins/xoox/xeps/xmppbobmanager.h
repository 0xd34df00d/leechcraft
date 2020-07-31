/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppClientExtension.h>
#include <QCache>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class XMPPBobIq;

	class XMPPBobManager : public QXmppClientExtension
	{
		Q_OBJECT

		QCache<QPair<QString, QString>, QByteArray> BobCache_;
	public:
		XMPPBobManager (unsigned int cacheSizeKb = 2048);

		bool handleStanza (const QDomElement&);
		QStringList discoveryFeatures () const;

		QString RequestBob (const QString&, const QString&);
		QByteArray Take (const QString&, const QString&);
	signals:
		void bobReceived (const XMPPBobIq&);
	};
}
}
}
