/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <qwebhistoryinterface.h>
#include <QSet>

namespace LC
{
namespace Poshuku
{
namespace WebKitView
{
	class LinkHistory : public QWebHistoryInterface
	{
		Q_OBJECT

		QSet<QString> History_;
	public:
		using QWebHistoryInterface::QWebHistoryInterface;

		void addHistoryEntry (const QString& url);
		bool historyContains (const QString& url) const;
	};
}
}
}
