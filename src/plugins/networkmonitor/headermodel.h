/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <QNetworkAccessManager>

namespace LC
{
	namespace Plugins
	{
		namespace NetworkMonitor
		{
			class HeaderModel : public QStandardItemModel
			{
				Q_OBJECT
			public:
				HeaderModel (QObject* = 0);
				void AddHeader (const QString&, const QString&);
			};
		}
	}
}
