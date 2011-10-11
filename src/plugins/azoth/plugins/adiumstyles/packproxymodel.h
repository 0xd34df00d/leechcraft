/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_ADIUMSTYLES_PACKPROXYMODEL_H
#define PLUGINS_AZOTH_PLUGINS_ADIUMSTYLES_PACKPROXYMODEL_H
#include <boost/shared_ptr.hpp>
#include <QStandardItemModel>
#include <QHash>

namespace LeechCraft
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

		boost::shared_ptr<Util::ResourceLoader> Loader_;

		struct OrigData
		{
			QList<QString> Variants_;
			QString Suffix_;
		};

		QHash<QString, OrigData> OrigDatas_;
	public:
		PackProxyModel (boost::shared_ptr<Util::ResourceLoader>, QObject* = 0);

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

#endif
