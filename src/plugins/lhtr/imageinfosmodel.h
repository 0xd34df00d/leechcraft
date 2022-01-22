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

class QNetworkReply;

namespace LC::LHTR
{
	class ImageInfosModel : public QAbstractItemModel
	{
		RemoteImageInfos_t& Infos_;
		const QStringList Columns_;
		QVector<QImage> Images_;

		ImageInfosModel& This_;

		enum Column
		{
			CImage,
			CSize,
			CAlt
		};
	public:
		ImageInfosModel (RemoteImageInfos_t& infos, QObject *parent);

		QModelIndex index (int row, int column, const QModelIndex& parent = QModelIndex()) const override;
		QModelIndex parent (const QModelIndex& child) const override;
		int rowCount (const QModelIndex& parent = QModelIndex()) const override;
		int columnCount (const QModelIndex& parent = QModelIndex()) const override;

		QVariant headerData (int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

		Qt::ItemFlags flags (const QModelIndex& index) const override;
		QVariant data (const QModelIndex& index, int role = Qt::DisplayRole) const override;
		bool setData (const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
	private:
		void FetchImage (int);
	};
}
