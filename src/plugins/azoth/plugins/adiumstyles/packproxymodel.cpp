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

#include "packproxymodel.h"
#include <QtDebug>
#include <util/resourceloader.h>

namespace LeechCraft
{
namespace Azoth
{
namespace AdiumStyles
{
	PackProxyModel::PackProxyModel (std::shared_ptr<Util::ResourceLoader> loader, QObject *parent)
	: QStandardItemModel (parent)
	, Loader_ (loader)
	{
		QAbstractItemModel *model = loader->GetSubElemModel ();

		connect (model,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (handleRowsInserted (QModelIndex, int, int)));

		connect (model,
				SIGNAL (rowsRemoved (QModelIndex, int, int)),
				this,
				SLOT (handleRowsRemoved (QModelIndex, int, int)));

		connect (model,
				SIGNAL (modelReset ()),
				this,
				SLOT (handleModelReset ()));

		handleModelReset ();
	}

	QString PackProxyModel::GetOrigName (const QString& pack) const
	{
		const QString& our = pack.split ('/', QString::SkipEmptyParts).value (0);
		if (!OrigDatas_.contains (our))
		{
			qWarning () << Q_FUNC_INFO
					<< "orig datas don't contain"
					<< our
					<< pack;
			return our;
		}

		return our + OrigDatas_ [our].Suffix_;
	}

	QString PackProxyModel::GetVariant (const QString& pack) const
	{
		return pack.split ('/', QString::SkipEmptyParts).value (1);
	}

	namespace
	{
		void DeSuf (QString& ourName, QString& suffix)
		{
			QStringList suffixes (".AdiumMessageStyle");

			Q_FOREACH (const QString& suf, suffixes)
				if (ourName.endsWith (suf))
				{
					ourName.chop (suf.length ());
					suffix = suf;
					break;
				}
		}
	}

	void PackProxyModel::handleRowsInserted (const QModelIndex& parent, int start, int end)
	{
		for (int i = start; i <= end; ++i)
		{
			const QString& origName = Loader_->GetSubElemModel ()->
					index (i, 0, parent).data ().toString ();

			OrigData origData;

			QString ourName = origName;
			DeSuf (ourName, origData.Suffix_);

			QStandardItem *item = new QStandardItem (ourName);
			item->setData (origName);
			appendRow (item);

			OrigDatas_ [ourName] = origData;
		}
	}

	void PackProxyModel::handleRowsRemoved (const QModelIndex& parent, int start, int end)
	{
		for (int i = start; i <= end; ++i)
		{
			const QString& origName = Loader_->GetSubElemModel ()->
					index (i, 0, parent).data ().toString ();

			QString ourName = origName;
			QString suf;
			DeSuf (ourName, suf);

			auto items = findItems (ourName);
			Q_FOREACH (QStandardItem *item, items)
				delete takeItem (item->row ());
		}
	}

	void PackProxyModel::handleModelReset ()
	{
		clear ();

		const int rc = Loader_->GetSubElemModel ()->rowCount ();
		if (rc)
			handleRowsInserted (QModelIndex (), 0, rc);
	}
}
}
}
