/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imageinfosmodel.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QImage>
#include <QtDebug>
#include <interfaces/core/icoreproxy.h>

namespace LC::LHTR
{
	ImageInfosModel::ImageInfosModel (RemoteImageInfos_t& infos, QObject* parent)
	: QAbstractItemModel { parent }
	, Infos_ (infos)
	, Columns_ { tr ("Image"), tr ("Size"), tr ("Alt") }
	, This_ { *this }
	{
		Images_.resize (infos.size ());
	}

	QModelIndex ImageInfosModel::index (int row, int column, const QModelIndex& parent) const
	{
		if (parent.isValid () || !hasIndex (row, column, parent))
			return {};

		return createIndex (row, column);
	}

	QModelIndex ImageInfosModel::parent (const QModelIndex&) const
	{
		return {};
	}

	int ImageInfosModel::rowCount (const QModelIndex& parent) const
	{
		return parent.isValid () ? 0 : Infos_.size ();
	}

	int ImageInfosModel::columnCount (const QModelIndex&) const
	{
		return Columns_.size ();
	}

	QVariant ImageInfosModel::headerData (int section, Qt::Orientation orientation, int role) const
	{
		return orientation != Qt::Horizontal || role != Qt::DisplayRole ?
				QVariant {} :
				Columns_.at (section);
	}

	Qt::ItemFlags ImageInfosModel::flags (const QModelIndex& index) const
	{
		auto flags = QAbstractItemModel::flags (index);
		if (index.column () == Column::CAlt)
			flags |= Qt::ItemFlag::ItemIsEditable;
		return flags;
	}

	QVariant ImageInfosModel::data (const QModelIndex& index, int role) const
	{
		const auto row = index.row ();
		const auto& info = Infos_.at (row);

		switch (static_cast<Column> (index.column ()))
		{
		case CImage:
			if (role != Qt::DecorationRole)
				return {};

			if (!Images_.at (row).isNull ())
				return Images_.at (row);

			This_.FetchImage (row);
			return {};
		case CSize:
			if (role != Qt::DisplayRole)
				return {};

			return QStringLiteral ("%1×%2")
					.arg (info.FullSize_.width ())
					.arg (info.FullSize_.height ());
		case CAlt:
			switch (role)
			{
			case Qt::DisplayRole:
			case Qt::EditRole:
				return info.Title_;
			default:
				return {};
			}
		}

		return {};
	}

	bool ImageInfosModel::setData (const QModelIndex& index, const QVariant& value, int)
	{
		if (index.column () != Column::CAlt)
			return false;

		Infos_ [index.row ()].Title_ = value.toString ();
		emit dataChanged (index, index);
		return true;
	}

	void ImageInfosModel::FetchImage (int row)
	{
		const auto& info = Infos_.at (row);

		const auto nam = GetProxyHolder ()->GetNetworkAccessManager ();
		for (const auto& url : { info.Thumb_, info.Preview_, info.Full_ })
		{
			if (!url.isValid ())
				continue;

			const auto reply = nam->get (QNetworkRequest { url });
			connect (reply,
					&QNetworkReply::finished,
					this,
					[this, reply, row]
					{
						reply->deleteLater ();

						QImage image;
						if (!image.loadFromData (reply->readAll ()))
						{
							qWarning () << "cannot read data from"
									<< reply->request ().url ();
							return;
						}

						Images_ [row] = image.scaledToHeight (128, Qt::SmoothTransformation);

						const auto& modelIdx = index (row, Column::CImage);
						emit dataChanged (modelIdx, modelIdx);
					});
		}
	}
}
