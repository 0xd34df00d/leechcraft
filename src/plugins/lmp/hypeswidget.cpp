/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hypeswidget.h"
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <util/xpc/util.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/standardnamfactory.h>
#include <util/qml/themeimageprovider.h>
#include <util/sll/prelude.h>
#include <util/sys/paths.h>
#include <util/models/rolenamesmixin.h>
#include <util/models/roleditemsmodel.h>
#include <util/threads/futures.h>
#include <interfaces/media/ihypesprovider.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iinfo.h>
#include "util.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LC::LMP
{
	struct HypesWidget::HypedTrack : Media::HypedTrackInfo
	{
		QString Stats_;
	};

	namespace
	{
		auto MakeTracksModel (QObject *parent)
		{
			using Util::RoledMemberField_v;
			return new HypesWidget::TracksModel
			{
				parent,
				RoledMemberField_v<"trackName", &HypesWidget::HypedTrack::TrackName_>,
				RoledMemberField_v<"trackURL", &HypesWidget::HypedTrack::TrackPage_>,
				RoledMemberField_v<"artistName", &HypesWidget::HypedTrack::ArtistName_>,
				RoledMemberField_v<"artistURL", &HypesWidget::HypedTrack::ArtistPage_>,
				RoledMemberField_v<"thumbImageURL", &HypesWidget::HypedTrack::Image_>,
				RoledMemberField_v<"fullURL", &HypesWidget::HypedTrack::LargeImage_>,
				RoledMemberField_v<"change", &HypesWidget::HypedTrack::Stats_>,
			};
		}
	}

	HypesWidget::HypesWidget (QWidget *parent)
	: QWidget (parent)
	, HypesView_ (new QQuickWidget)
	, NewArtistsModel_ (MakeSimilarModel (this))
	, TopArtistsModel_ (MakeSimilarModel (this))
	, NewTracksModel_ (MakeTracksModel (this))
	, TopTracksModel_ (MakeTracksModel (this))
	{
		Ui_.setupUi (this);
		layout ()->addWidget (HypesView_);

		HypesView_->setResizeMode (QQuickWidget::SizeRootObjectToView);

		HypesView_->engine ()->addImageProvider ("ThemeIcons", new Util::ThemeImageProvider (GetProxyHolder ()));

		new Util::StandardNAMFactory ("lmp/qml",
				[] { return 50 * 1024 * 1024; },
				HypesView_->engine ());

		auto root = HypesView_->rootContext ();
		root->setContextProperty ("newArtistsModel", NewArtistsModel_);
		root->setContextProperty ("newTracksModel", NewTracksModel_);
		root->setContextProperty ("topArtistsModel", TopArtistsModel_);
		root->setContextProperty ("topTracksModel", TopTracksModel_);
		root->setContextProperty ("artistsLabelText", tr ("Hyped artists"));
		root->setContextProperty ("tracksLabelText", tr ("Hyped tracks"));
		root->setContextProperty ("newsText", tr ("Show novelties"));
		root->setContextProperty ("topsText", tr ("Show tops"));
		root->setContextProperty ("colorProxy", new Util::ColorThemeProxy (GetProxyHolder ()->GetColorThemeManager (), this));

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, ""))
			HypesView_->engine ()->addImportPath (cand);

		HypesView_->setSource (Util::GetSysPathUrl (Util::SysPath::QML, "lmp", "HypesView.qml"));

		connect (Ui_.InfoProvider_,
				SIGNAL (activated (int)),
				this,
				SLOT (request ()));

		connect (HypesView_->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLink (QString)));
		connect (HypesView_->rootObject (),
				SIGNAL (artistPreviewRequested (QString)),
				this,
				SIGNAL (artistPreviewRequested (QString)));
		connect (HypesView_->rootObject (),
				SIGNAL (trackPreviewRequested (QString, QString)),
				this,
				SIGNAL (trackPreviewRequested (QString, QString)));

		connect (HypesView_->rootObject (),
				SIGNAL (browseInfo (QString)),
				&Core::Instance (),
				SIGNAL (artistBrowseRequested (QString)));
	}

	void HypesWidget::InitializeProviders ()
	{
		const auto& lastProv = ShouldRememberProvs () ?
				XmlSettingsManager::Instance ()
					.Property ("LastUsedHypesProvider", QString ()).toString () :
				QString ();

		bool lastFound = false;

		Providers_ = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableRoots<Media::IHypesProvider*> ();
		for (auto provObj : Providers_)
		{
			auto prov = qobject_cast<Media::IHypesProvider*> (provObj);

			Ui_.InfoProvider_->addItem (qobject_cast<IInfo*> (provObj)->GetIcon (),
					prov->GetServiceName ());
			if (prov->GetServiceName () == lastProv)
			{
				const int idx = Providers_.size () - 1;
				Ui_.InfoProvider_->setCurrentIndex (idx);
				request ();
				lastFound = true;
			}
		}

		if (!lastFound)
			Ui_.InfoProvider_->setCurrentIndex (-1);
	}

	namespace
	{
		template<Media::IHypesProvider::HypeType Type, auto Handler, typename C>
		void TryHype (C *ctx, Media::IHypesProvider *prov)
		{
			if (!prov->SupportsHype (Type))
				return;

			Util::Sequence (ctx, prov->RequestHype (Type)) >>
					Util::Visitor
					{
						[] (const QString&) { /* TODO */ },
						[=] (const auto& res) { std::invoke (Handler, ctx, Media::GetHypedInfo<Type> (res), Type); }
					};
		}
	}

	void HypesWidget::request ()
	{
		NewArtistsModel_->SetItems ({});
		TopArtistsModel_->SetItems ({});
		NewTracksModel_->SetItems ({});
		TopTracksModel_->SetItems ({});

		const auto idx = Ui_.InfoProvider_->currentIndex ();
		if (idx < 0)
			return;

		for (auto prov : Providers_)
			disconnect (prov,
					0,
					this,
					0);

		auto provObj = Providers_.at (idx);
		auto prov = qobject_cast<Media::IHypesProvider*> (provObj);
		TryHype<Media::IHypesProvider::HypeType::NewArtists, &HypesWidget::HandleArtists> (this, prov);
		TryHype<Media::IHypesProvider::HypeType::TopArtists, &HypesWidget::HandleArtists> (this, prov);
		TryHype<Media::IHypesProvider::HypeType::NewTracks, &HypesWidget::HandleTracks> (this, prov);
		TryHype<Media::IHypesProvider::HypeType::TopTracks, &HypesWidget::HandleTracks> (this, prov);

		XmlSettingsManager::Instance ().setProperty ("LastUsedHypesProvider", prov->GetServiceName ());
	}

	namespace
	{
		template<typename T>
		QStringList GetStats (const T& info)
		{
			QStringList stats;
			if (info.PercentageChange_)
				stats << HypesWidget::tr ("Growth: x%1", "better use unicode multiplication sign here instead of 'x'")
						.arg (info.PercentageChange_ / 100., 0, 'f', 2);
			if (info.Listeners_)
				stats << HypesWidget::tr ("%n listener(s)", 0, info.Listeners_);
			if (info.Playcount_)
				stats << HypesWidget::tr ("%n playback(s)", 0, info.Playcount_);
			return stats;
		}
	}

	void HypesWidget::HandleArtists (const QList<Media::HypedArtistInfo>& infos, Media::IHypesProvider::HypeType type)
	{
		auto model = type == Media::IHypesProvider::HypeType::NewArtists ?
				NewArtistsModel_ :
				TopArtistsModel_;
		model->SetItems (Util::MapAs<QVector> (infos,
				[] (const Media::HypedArtistInfo& info)
				{
					SimilarArtistInfo prepared { info.Info_, *Core::Instance ().GetLocalCollection () };
					if (prepared.ShortDesc_.isEmpty ())
						prepared.ShortDesc_ = tr ("%1 is not <em>that</em> mainstream to have a description.")
								.arg (info.Info_.Name_);
					prepared.Similarity_ = GetStats (info).join ("; ");
					return prepared;
				}));
	}

	void HypesWidget::HandleTracks (const QList<Media::HypedTrackInfo>& infos, Media::IHypesProvider::HypeType type)
	{
		auto model = type == Media::IHypesProvider::HypeType::NewTracks ?
				NewTracksModel_ :
				TopTracksModel_;
		model->SetItems (Util::MapAs<QVector> (infos,
				[] (const Media::HypedTrackInfo& info) { return HypedTrack { info, GetStats (info).join ("; ") }; }));
	}

	void HypesWidget::handleLink (const QString& link)
	{
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (Util::MakeEntity (QUrl (link),
					QString (),
					FromUserInitiated | OnlyHandle));
	}
}
