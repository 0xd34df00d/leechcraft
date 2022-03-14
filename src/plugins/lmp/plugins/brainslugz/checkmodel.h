/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStandardItemModel>
#include <QHash>
#include <QSet>
#include <util/models/rolenamesmixin.h>
#include <interfaces/lmp/collectiontypes.h>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/core/icoreproxy.h>

namespace Media
{
struct ReleaseInfo;
class IAlbumArtProvider;
class IArtistBioFetcher;
}

namespace LC::LMP::BrainSlugz
{
	class CheckModel : public Util::RoleNamesMixin<QStandardItemModel>
	{
		Q_OBJECT

		QHash<int, QStandardItem*> Artist2Item_;
		QHash<int, QStandardItemModel*> Artist2Submodel_;

		QSet<int> Scheduled_;

		const Collection::Artists_t AllArtists_;
		const ILMPProxy_ptr Proxy_;
		const QString DefaultAlbumIcon_;
		const QString DefaultArtistIcon_;

		Media::IAlbumArtProvider * const AAProv_;
		Media::IArtistBioFetcher * const BioProv_;
	public:
		enum Role
		{
			ArtistId = Qt::UserRole + 1,
			ArtistName,
			ScheduledToCheck,
			IsChecked,
			ArtistImage,
			Releases,
			MissingCount,
			PresentCount
		};

		CheckModel (const Collection::Artists_t&,
				const ICoreProxy_ptr&, const ILMPProxy_ptr&, QObject*);

		Collection::Artists_t GetSelectedArtists () const;

		void SetMissingReleases (const QList<Media::ReleaseInfo>&, const Collection::Artist&);
		void MarkNoNews (const Collection::Artist&);

		void RemoveUnscheduled ();

		Q_INVOKABLE void SelectAll ();
		Q_INVOKABLE void SelectNone ();
		Q_INVOKABLE void SetArtistScheduled (int, bool);
	};
}
