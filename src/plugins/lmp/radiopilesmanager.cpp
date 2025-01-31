/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "radiopilesmanager.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <QtDebug>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/iaudiopile.h>
#include <interfaces/media/iradiostationprovider.h>
#include "util/lmp/mediainfo.h"
#include "previewcharacteristicinfo.h"
#include "util.h"

namespace LC
{
namespace LMP
{
	RadioPilesManager::RadioPilesManager (const IPluginsManager *pm, QObject *parent)
	: QObject { parent }
	, PilesModel_ { new QStandardItemModel { this } }
	{
		FillModel (pm);
	}

	QAbstractItemModel* RadioPilesManager::GetModel () const
	{
		return PilesModel_;
	}

	void RadioPilesManager::FillModel (const IPluginsManager *pm)
	{
		for (auto pileObj : pm->GetAllCastableRoots<Media::IAudioPile*> ())
		{
			auto pile = qobject_cast<Media::IAudioPile*> (pileObj);

			auto item = new QStandardItem (tr ("Search in %1")
					.arg (pile->GetServiceName ()));
			item->setIcon (pile->GetServiceIcon ());
			item->setEditable (false);

			const auto function = [item, pile, this] { HandlePile (item, pile); };
			item->setData (QVariant::fromValue<Media::ActionFunctor_f> (function),
					Media::RadioItemRole::ActionFunctor);

			PilesModel_->appendRow (item);
		}
	}

	namespace
	{
		void AddResults (const Media::IAudioPile::Results_t& results, QStandardItem *item)
		{
			const auto& queryText = item->text ();
			item->setText (RadioPilesManager::tr ("%1 (%n result(s))", 0, results.size ())
						.arg (queryText));

			QSet<QUrl> urls;
			QSet<PreviewCharacteristicInfo> infos;
			for (const auto& res : results)
			{
				if (urls.contains (res.Source_))
					continue;
				urls.insert (res.Source_);

				const PreviewCharacteristicInfo checkInfo { res.Info_ };
				if (infos.contains (checkInfo))
					continue;
				infos << checkInfo;

				auto info = res.Info_;
				info.Other_ ["URL"] = res.Source_;

				const auto& name = PerformSubstitutionsPlaylist (MediaInfo::FromAudioInfo (info));
				const auto resItem = new QStandardItem { name };
				resItem->setEditable (false);
				resItem->setData ({}, Media::RadioItemRole::ItemType);
				resItem->setData (Media::RadioType::SingleTrack, Media::RadioItemRole::ItemType);
				resItem->setData (QVariant::fromValue<QList<Media::AudioInfo>> ({ info }),
						Media::RadioItemRole::TracksInfos);
				item->appendRow (resItem);
			}
		}
	}

	void RadioPilesManager::HandlePile (QStandardItem *item, Media::IAudioPile *pile)
	{
		const auto& query = QInputDialog::getText (nullptr,
				tr ("Audio search"),
				tr ("Enter the string to search for:"));
		if (query.isEmpty ())
			return;

		Media::AudioSearchRequest req;
		req.FreeForm_ = query;

		const auto searchItem = new QStandardItem { query };
		searchItem->setData (Media::RadioType::TracksRoot, Media::RadioItemRole::ItemType);
		searchItem->setEditable (false);
		item->appendRow (searchItem);

		Util::Sequence (this, pile->Search (req)) >>
				Util::Visitor
				{
					[] (const QString&) { /* TODO */ },
					[searchItem] (const Media::IAudioPile::Results_t& results)
					{
						AddResults (results, searchItem);
					}
				};
	}
}
}
