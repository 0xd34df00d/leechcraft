/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "streamlistfetcherbase.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardItem>
#include <interfaces/media/iradiostationprovider.h>

namespace LeechCraft
{
namespace HotStreams
{
	StreamListFetcherBase::StreamListFetcherBase (QStandardItem *root, QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, Root_ (root)
	{
	}

	void StreamListFetcherBase::Request (const QNetworkRequest& req)
	{
		auto reply = NAM_->get (req);
		connect (reply,
				SIGNAL (finished ()),
				this,
				SLOT (handleReplyFinished ()));
	}

	void StreamListFetcherBase::HandleData (const QByteArray& data)
	{
		for (const auto& stream : Parse (data))
		{
			auto name = stream.Name_;
			if (!stream.Genres_.isEmpty ())
				name += " (" + stream.Genres_.join ("; ") + ")";

			auto item = new QStandardItem (name);
			item->setToolTip (stream.Description_);
			item->setData (Media::RadioType::Predefined, Media::RadioItemRole::ItemType);
			item->setData (stream.URL_, Media::RadioItemRole::RadioID);
			item->setEditable (false);
			Root_->appendRow (item);
		}

		deleteLater ();
	}

	void StreamListFetcherBase::handleReplyFinished ()
	{
		auto reply = qobject_cast<QNetworkReply*> (sender ());
		if (!reply)
			return;

		reply->deleteLater ();
		HandleData (reply->readAll ());
	}
}
}
