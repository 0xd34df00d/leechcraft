/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QAbstractItemModel>
#include <QStringList>
#include <QVector>
#include <interfaces/data/iimgsource.h>
#include <interfaces/core/icoreproxy.h>

namespace LC
{
namespace LHTR
{
	class ImageInfosModel : public QAbstractItemModel
	{
		Q_OBJECT

		const ICoreProxy_ptr Proxy_;
		RemoteImageInfos_t& Infos_;
		const QStringList Columns_;

		QVector<QImage> Images_;
		mutable QMap<QNetworkReply*, int> Reply2Image_;

		enum Column
		{
			CImage,
			CSize,
			CAlt
		};
	public:
		ImageInfosModel (RemoteImageInfos_t& infos, ICoreProxy_ptr, QObject *parent);

		QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex()) const;
		QModelIndex parent (const QModelIndex& child) const;
		int rowCount (const QModelIndex& parent = QModelIndex()) const;
		int columnCount (const QModelIndex& parent = QModelIndex()) const;

		QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

		Qt::ItemFlags flags (const QModelIndex& index) const;
		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const;
		bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
	private:
		void FetchImage (int) const;
	private slots:
		void handleImageFetched ();
	};
}
}
