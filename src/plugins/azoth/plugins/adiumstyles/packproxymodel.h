/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QStandardItemModel>
#include <QHash>

namespace LC
{
namespace Util
{
	class ResourceLoader;
}

namespace Azoth
{
namespace AdiumStyles
{
	class PackProxyModel : public QStandardItemModel
	{
		Q_OBJECT

		std::shared_ptr<Util::ResourceLoader> Loader_;

		struct OrigData
		{
			QList<QString> Variants_;
			QString Suffix_;
		};

		QHash<QString, OrigData> OrigDatas_;
	public:
		PackProxyModel (std::shared_ptr<Util::ResourceLoader>, QObject* = 0);

		QString GetOrigName (const QString&) const;
		QString GetVariant (const QString&) const;
	private slots:
		void handleRowsInserted (const QModelIndex&, int, int);
		void handleRowsRemoved (const QModelIndex&, int, int);
		void handleModelReset ();
	};
}
}
}
