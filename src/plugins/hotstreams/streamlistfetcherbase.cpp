/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "streamlistfetcherbase.h"
#include <functional>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QStandardItem>
#include <QtConcurrentRun>
#include <util/sll/prelude.h>
#include <util/threads/futures.h>
#include <interfaces/media/iradiostationprovider.h>
#include "roles.h"

namespace LC
{
namespace HotStreams
{
	StreamListFetcherBase::StreamListFetcherBase (QStandardItem *root, QNetworkAccessManager *nam, QObject *parent)
	: QObject (parent)
	, NAM_ (nam)
	, Root_ (root)
	, RadioIcon_ (":/hotstreams/resources/images/radio.png")
	{
	}

	void StreamListFetcherBase::Request (const QNetworkRequest& req)
	{
		auto reply = NAM_->get (req);
		connect (reply,
				&QNetworkReply::finished,
				this,
				[this, reply]
				{
					reply->deleteLater ();
					HandleData (reply->readAll ());
				});
	}

	void StreamListFetcherBase::HandleData (const QByteArray& data)
	{
		const auto future = QtConcurrent::run ([this, data]
				{
					auto result = Parse (data);
					std::sort (result.begin (), result.end (), Util::ComparingBy (&StreamInfo::Name_));
					return result;
				});
		Util::Sequence (this, future) >>
				[this] (const auto& streams)
				{
					for (const auto& stream : streams)
					{
						auto name = stream.Name_;
						if (!stream.Genres_.isEmpty ())
							name += " (" + stream.Genres_.join ("; ") + ")";

						auto tooltip = "<span style=\"white-space: nowrap\">" + stream.Description_;
						if (!stream.DJ_.isEmpty ())
							tooltip += "<br /><em>DJ:</em> " + stream.DJ_;
						tooltip += "</span>";

						auto item = new QStandardItem (name);
						item->setToolTip (tooltip);
						item->setIcon (RadioIcon_);
						item->setData (stream.Name_, StreamItemRoles::PristineName);
						item->setData (Media::RadioType::Predefined, Media::RadioItemRole::ItemType);
						item->setData (stream.URL_, Media::RadioItemRole::RadioID);
						item->setData (stream.PlaylistFormat_, StreamItemRoles::PlaylistFormat);
						item->setEditable (false);
						Root_->appendRow (item);
					}

					deleteLater ();
				};
	}
}
}
