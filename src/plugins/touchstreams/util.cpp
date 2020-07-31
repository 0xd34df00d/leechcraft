/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QStandardItem>
#include <util/sll/urloperator.h>
#include <util/svcauth/vkauthmanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/media/iradiostationprovider.h>
#include <interfaces/media/audiostructs.h>

namespace LC
{
namespace TouchStreams
{
	bool CheckAuthentication (QStandardItem *rootItem,
			Util::SvcAuth::VkAuthManager *authMgr, const ICoreProxy_ptr& proxy)
	{
		if (authMgr->HadAuthentication ())
			return true;

		auto item = new QStandardItem (QObject::tr ("Authenticate"));
		item->setEditable (false);
		item->setIcon (proxy->GetIconThemeManager ()->GetIcon ("emblem-locked"));
		item->setData ("auth", Media::RadioItemRole::RadioID);
		item->setData (Media::RadioType::RadioAction, Media::RadioItemRole::ItemType);
		rootItem->appendRow (item);
		return false;
	}

	std::optional<Media::AudioInfo> TrackMap2Info (const QVariantMap& map)
	{
		const auto& url = QUrl::fromEncoded (map ["url"].toString ().toUtf8 ());
		if (!url.isValid ())
			return {};

		Media::AudioInfo info;
		info.Title_ = map ["title"].toString ();
		info.Artist_ = map ["artist"].toString ();
		info.Length_ = map ["duration"].toInt ();
		info.Other_ ["URL"] = url;
		return info;
	}

	QString TrackMap2RadioId (const QVariantMap& map)
	{
		QUrl radioID { "vk://track" };
		Util::UrlOperator { radioID }
				("audio_id", map.value ("id").toString ())
				("owner_id", map.value ("owner_id").toString ());

		return radioID.toString (QUrl::FullyEncoded);
	}
}
}
