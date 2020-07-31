/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imageinfosmodel.h"
#include <functional>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QImage>
#include <QtDebug>

namespace LC
{
namespace LHTR
{
	ImageInfosModel::ImageInfosModel (RemoteImageInfos_t& infos, ICoreProxy_ptr proxy, QObject* parent)
	: QAbstractItemModel { parent }
	, Proxy_ { proxy }
	, Infos_ (infos)
	, Columns_ { tr ("Image"), tr ("Size"), tr ("Alt") }
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
		const auto& info = Infos_.at (index.row ());

		const std::map<Column, std::map<int, std::function<QVariant ()>>> map
		{
			{
				CImage,
				{
					{
						Qt::DecorationRole,
						[this, &index] () -> QVariant
						{
							if (!Images_.at (index.row ()).isNull ())
								return Images_.at (index.row ());

							FetchImage (index.row ());
							return {};
						}
					}
				}
			},
			{
				CSize,
				{
					{
						Qt::DisplayRole,
						[&info]
						{
							return QString::fromUtf8 ("%1×%2")
									.arg (info.FullSize_.width ())
									.arg (info.FullSize_.height ());
						}
					}
				}
			},
			{
				CAlt,
				{
					{
						Qt::DisplayRole,
						[&info] { return info.Title_; }
					},
					{
						Qt::EditRole,
						[&info] { return info.Title_; }
					}
				}
			}
		};

		const auto& roleMap = map.find (static_cast<Column> (index.column ()))->second;
		const auto& roleIt = roleMap.find (role);
		return roleIt == roleMap.end () ? QVariant {} : roleIt->second ();
	}

	bool ImageInfosModel::setData (const QModelIndex& index, const QVariant& value, int)
	{
		if (index.column () != Column::CAlt)
			return false;

		Infos_ [index.row ()].Title_ = value.toString ();
		emit dataChanged (index, index);
		return true;
	}

	void ImageInfosModel::FetchImage (int row) const
	{
		const auto& info = Infos_.at (row);

		const auto nam = Proxy_->GetNetworkAccessManager ();
		for (const auto& url : { info.Thumb_, info.Preview_, info.Full_ })
		{
			if (!url.isValid ())
				continue;

			const auto reply = nam->get (QNetworkRequest { url });
			Reply2Image_ [reply] = row;
			connect (reply,
					SIGNAL (finished ()),
					this,
					SLOT (handleImageFetched ()));
		}
	}

	void ImageInfosModel::handleImageFetched ()
	{
		const auto reply = qobject_cast<QNetworkReply*> (sender ());
		const auto idx = Reply2Image_.take (reply);

		QImage image;
		if (!image.loadFromData (reply->readAll ()))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot read data from"
					<< reply->request ().url ();
			return;
		}

		Images_ [idx] = image.scaledToHeight (128, Qt::SmoothTransformation);

		const auto& modelIdx = index (idx, Column::CImage);
		emit dataChanged (modelIdx, modelIdx);
	}
}
}
