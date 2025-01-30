/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "playlistmodel.h"
#include <QMimeData>
#include <QFileInfo>
#include <util/sll/prelude.h>
#include "util/lmp/util.h"
#include "playlistparsers/playlistfactory.h"
#include "player.h"
#include "util.h"
#include "core.h"
#include "radiomanager.h"

namespace LC
{
namespace LMP
{
	PlaylistModel::PlaylistModel (Player *parent)
	: DndActionsMixin<QStandardItemModel> (parent)
	, Player_ (parent)
	{
		setSupportedDragActions (Qt::CopyAction | Qt::MoveAction);
	}

	QStringList PlaylistModel::mimeTypes () const
	{
		return { "text/uri-list" };
	}

	QMimeData* PlaylistModel::mimeData (const QModelIndexList& indexes) const
	{
		QList<QUrl> urls;
		for (const auto& index : indexes)
			urls += Util::Map (Player_->GetIndexSources (index), &AudioSource::ToUrl);
		urls.removeAll ({});

		const auto result = new QMimeData;
		result->setUrls (urls);
		return result;
	}

	namespace
	{
		QList<AudioSource> GetSources (const QMimeData *data)
		{
			QList<AudioSource> sources;
			for (const auto& url : data->urls ())
			{
				if (url.scheme () != "file")
				{
					sources << AudioSource (url);
					continue;
				}

				const auto& localPath = url.toLocalFile ();
				if (QFileInfo (localPath).isFile ())
				{
					sources << AudioSource (localPath);
					continue;
				}

				for (const auto& path : RecIterate (localPath, true))
					sources << AudioSource (path);
			}

			return sources;
		}

		QList<MediaInfo> GetInfos (const QMimeData *data)
		{
			const auto& serialized = data->data ("x-leechcraft-lmp/media-info-list");
			if (serialized.isEmpty ())
				return {};

			QDataStream stream { serialized };
			QList<MediaInfo> result;
			stream >> result;
			return result;
		}
	}

	bool PlaylistModel::dropMimeData (const QMimeData *data,
			Qt::DropAction action, int row, int, const QModelIndex& parent)
	{
		if (action == Qt::IgnoreAction)
			return true;

		if (data->hasUrls ())
			HandleDroppedUrls (data, row, parent);

		HandleRadios (data);

		return true;
	}

	Qt::DropActions PlaylistModel::supportedDropActions () const
	{
		return Qt::CopyAction | Qt::MoveAction;
	}

	void PlaylistModel::HandleRadios (const QMimeData *data)
	{
		QStringList radioIds;

		QDataStream stream { data->data ("x-leechcraft-lmp/radio-ids") };
		stream >> radioIds;

		for (const auto& radioId : radioIds)
			if (const auto station = Core::Instance ().GetRadioManager ()->GetRadioStation (radioId))
			{
				Player_->SetRadioStation (station);
				break;
			}
	}

	void PlaylistModel::HandleDroppedUrls (const QMimeData *data, int row, const QModelIndex& parent)
	{
		const auto& sources = GetSources (data);
		const auto& infos = GetInfos (data);

		if (infos.size () == sources.size ())
			for (int i = 0; i < sources.size (); ++i)
				Player_->PrepareURLInfo (sources.at (i).ToUrl (), infos.at (i));

		auto afterIdx = row >= 0 ?
				index (row, 0, parent) :
				parent;
		const auto& firstSrc = afterIdx.isValid () ?
				Player_->GetIndexSources (afterIdx).value (0) :
				AudioSource ();

		auto existingQueue = Player_->GetQueue ();
		for (const auto& src : sources)
		{
			auto remPos = std::remove (existingQueue.begin (), existingQueue.end (), src);
			existingQueue.erase (remPos, existingQueue.end ());
		}

		auto pos = std::find (existingQueue.begin (), existingQueue.end (), firstSrc);
		if (pos == existingQueue.end ())
			existingQueue << sources;
		else
		{
			for (const auto& src : sources)
				pos = existingQueue.insert (pos, src) + 1;
		}

		Player_->Enqueue (existingQueue, Player::EnqueueReplace | Player::EnqueueSort);
	}
}
}
