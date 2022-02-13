/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "similarviewmanager.h"
#include <algorithm>
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <util/models/roleditemsmodel.h>
#include <util/qml/colorthemeproxy.h>
#include <util/sys/paths.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/sll/prelude.h>
#include <util/threads/futures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/isimilarartists.h>
#include "core.h"
#include "stdartistactionsmanager.h"

namespace LC
{
namespace LMP
{
	SimilarViewManager::SimilarViewManager (QQuickWidget *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Model_ (MakeSimilarModel (this))
	{
		View_->rootContext ()->setContextProperty ("similarModel", Model_);
		View_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (GetProxyHolder ()->GetColorThemeManager (), this));

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			View_->engine ()->addImportPath (cand);

		new StdArtistActionsManager { *View_, this };
	}

	void SimilarViewManager::DefaultRequest (const QString& artist)
	{
		const auto& similars = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<Media::ISimilarArtists*> ();
		for (auto similar : similars)
			Util::Sequence (this, similar->GetSimilarArtists (artist, 20)) >>
					Util::Visitor
					{
						[] (const QString&) {},
						[this] (const Media::SimilarityInfos_t& infos) { SetInfos (infos); }
					};
	}

	void SimilarViewManager::SetInfos (Media::SimilarityInfos_t infos)
	{
		std::sort (infos.begin (), infos.end (),
				[] (const Media::SimilarityInfo& left, const Media::SimilarityInfo& right)
					{ return left.Similarity_ > right.Similarity_; });

		Model_->SetItems (Util::MapAs<QVector> (infos,
				[] (const Media::SimilarityInfo& info)
				{
					SimilarArtistInfo artistInfo { info.Artist_, *Core::Instance ().GetLocalCollection () };
					if (info.Similarity_ > 0)
						artistInfo.Similarity_ = tr ("Similarity: %1%")
								.arg (info.Similarity_);
					else if (!info.SimilarTo_.isEmpty ())
						artistInfo.Similarity_ = tr ("Similar to: %1")
								.arg (info.SimilarTo_.join ("; "));
					return artistInfo;
				}));
	}
}
}
