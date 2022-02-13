/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "recommendationswidget.h"
#include <QtDebug>
#include <util/sll/prelude.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/irecommendedartists.h>
#include <interfaces/media/iaudioscrobbler.h>
#include <interfaces/iinfo.h>
#include "xmlsettingsmanager.h"
#include "util.h"
#include "similarview.h"

namespace LC
{
namespace LMP
{
	RecommendationsWidget::RecommendationsWidget (QWidget *parent)
	: QWidget { parent }
	, RecView_ { new SimilarView }
	{
		Ui_.setupUi (this);
		layout ()->addWidget (RecView_);
	}

	void RecommendationsWidget::InitializeProviders ()
	{
		for (auto prov : GetProxyHolder ()->GetPluginsManager ()->GetAllCastableTo<Media::IRecommendedArtists*> ())
		{
			Util::Sequence (this, prov->RequestRecommended (10)) >>
					Util::Visitor
					{
						[] (const QString&) {},
						[this] (const Media::SimilarityInfos_t& similars) { HandleInfos (similars); }
					};
		}
	}

	void RecommendationsWidget::HandleInfos (const Media::SimilarityInfos_t& similars)
	{
		const auto initSize = Similars_.size ();
		Similars_ += similars;

		if (initSize)
			std::inplace_merge (Similars_.begin (), Similars_.begin () + initSize, Similars_.end (),
					Util::ComparingBy (&Media::SimilarityInfo::Similarity_));

		RecView_->SetSimilarArtists (Similars_);
	}
}
}
