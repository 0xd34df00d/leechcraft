/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QXmppClientExtension.h>

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class LastActivityManager : public QXmppClientExtension
	{
		Q_OBJECT
	public:
		QStringList discoveryFeatures () const;
		bool handleStanza (const QDomElement&);

		QString RequestLastActivity (const QString&);
	private:
		QXmppIq CreateIq (const QString&, int = -1);
	signals:
		void gotLastActivity (const QString&, int);
	};
}
}
}
