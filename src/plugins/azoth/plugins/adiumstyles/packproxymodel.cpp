/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "packproxymodel.h"
#include <QtDebug>
#include <util/sys/resourceloader.h>

namespace LC
{
namespace Azoth
{
namespace AdiumStyles
{
	PackProxyModel::PackProxyModel (std::shared_ptr<Util::ResourceLoader> loader, QObject *parent)
	: QStandardItemModel (parent)
	, Loader_ (loader)
	{
		const auto model = loader->GetSubElemModel ();

		connect (model,
				SIGNAL (rowsInserted (QModelIndex, int, int)),
				this,
				SLOT (handleRowsInserted (QModelIndex, int, int)));

		connect (model,
				SIGNAL (rowsAboutToBeRemoved (QModelIndex, int, int)),
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
		const auto& our = pack.split ('/', Qt::SkipEmptyParts).value (0);
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
		return pack.split ('/', Qt::SkipEmptyParts).value (1);
	}

	namespace
	{
		void DeSuf (QString& ourName, QString& suffix)
		{
			const QStringList suffixes { ".AdiumMessageStyle" };

			for (const auto& suf : suffixes)
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
			const auto& origName = Loader_->GetSubElemModel ()->
					index (i, 0, parent).data ().toString ();

			OrigData origData;

			auto ourName = origName;
			DeSuf (ourName, origData.Suffix_);

			const auto item = new QStandardItem (ourName);
			item->setData (origName);
			appendRow (item);

			OrigDatas_ [ourName] = origData;
		}
	}

	void PackProxyModel::handleRowsRemoved (const QModelIndex& parent, int start, int end)
	{
		for (int i = start; i <= end; ++i)
		{
			auto ourName = Loader_->GetSubElemModel ()->
					index (i, 0, parent).data ().toString ();

			QString suf;
			DeSuf (ourName, suf);

			for (const auto item : findItems (ourName))
				removeRow (item->row ());
		}
	}

	void PackProxyModel::handleModelReset ()
	{
		clear ();

		if (const int rc = Loader_->GetSubElemModel ()->rowCount ())
			handleRowsInserted ({}, 0, rc);
	}
}
}
}
