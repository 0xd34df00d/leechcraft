/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#pragma once

#include <QSortFilterProxyModel>
#include <QStringList>

namespace LeechCraft
{
namespace Launchy
{
	class ItemsSortFilterProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT
		Q_PROPERTY (QString appFilterText READ GetAppFilterText WRITE SetAppFilterText NOTIFY appFilterTextChanged);

		QStringList CategoryNames_;
		QString AppFilterText_;
	public:
		ItemsSortFilterProxyModel (QAbstractItemModel*, QObject* = 0);

		QString GetAppFilterText () const;
		void SetAppFilterText (const QString&);
	protected:
		bool filterAcceptsRow (int, const QModelIndex&) const;
	public slots:
		void setCategoryNames (const QStringList&);
	private slots:
		void invalidateFilterSlot ();
	signals:
		void appFilterTextChanged ();
	};
}
}
