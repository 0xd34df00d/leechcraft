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
#include <util/qml/colorthemeproxy.h>
#include <util/sys/paths.h>
#include <util/sll/visitor.h>
#include <util/sll/either.h>
#include <util/threads/futures.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/isimilarartists.h>
#include "similarmodel.h"
#include "core.h"
#include "stdartistactionsmanager.h"

namespace LC
{
namespace LMP
{
	SimilarViewManager::SimilarViewManager (const ICoreProxy_ptr& proxy,
			QQuickWidget *view, QObject *parent)
	: QObject (parent)
	, View_ (view)
	, Model_ (new SimilarModel (this))
	, Proxy_ (proxy)
	{
		View_->rootContext ()->setContextProperty ("similarModel", Model_);
		View_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (Core::Instance ().GetProxy ()->GetColorThemeManager (), this));

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			View_->engine ()->addImportPath (cand);
	}

	void SimilarViewManager::InitWithSource ()
	{
		new StdArtistActionsManager (Proxy_, View_, this);
	}

	void SimilarViewManager::DefaultRequest (const QString& artist)
	{
		auto similars = Core::Instance ().GetProxy ()->
					GetPluginsManager ()->GetAllCastableTo<Media::ISimilarArtists*> ();
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
		Model_->clear ();

		std::sort (infos.begin (), infos.end (),
				[] (const Media::SimilarityInfo& left, const Media::SimilarityInfo& right)
					{ return left.Similarity_ > right.Similarity_; });

		for (const auto& info : infos)
		{
			auto item = SimilarModel::ConstructItem (info.Artist_);

			QString simStr;
			if (info.Similarity_ > 0)
				simStr = tr ("Similarity: %1%")
					.arg (info.Similarity_);
			else if (!info.SimilarTo_.isEmpty ())
				simStr = tr ("Similar to: %1")
					.arg (info.SimilarTo_.join ("; "));
			if (!simStr.isEmpty ())
				item->setData (simStr, SimilarModel::Role::Similarity);

			Model_->appendRow (item);
		}
	}
}
}
