/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chartswidget.h"
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <util/xpc/util.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/standardnamfactory.h>
#include <util/qml/themeimageprovider.h>
#include <util/sll/prelude.h>
#include <util/sll/udls.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include <util/models/rolenamesmixin.h>
#include <util/models/itemsmodel.h>
#include <util/threads/futures.h>
#include <interfaces/media/itopprovider.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/iinfo.h>
#include "util.h"
#include "xmlsettingsmanager.h"
#include "stdartistactionsmanager.h"
#include "literals.h"
#include "core.h"

namespace LC::LMP
{
	struct ChartsWidget::TopTrack : Media::TopTrackInfo
	{
		QString Stats_;
	};

	namespace
	{
		auto MakeTracksModel (QObject *parent)
		{
			using Util::NamedMemberField_v;
			return new ChartsWidget::TracksModel
			{
				parent,
				NamedMemberField_v<"trackName", &ChartsWidget::TopTrack::TrackName_>,
				NamedMemberField_v<"trackURL", &ChartsWidget::TopTrack::TrackPage_>,
				NamedMemberField_v<"artistName", &ChartsWidget::TopTrack::ArtistName_>,
				NamedMemberField_v<"artistURL", &ChartsWidget::TopTrack::ArtistPage_>,
				NamedMemberField_v<"thumbImageURL", &ChartsWidget::TopTrack::Image_>,
				NamedMemberField_v<"fullURL", &ChartsWidget::TopTrack::LargeImage_>,
				NamedMemberField_v<"stats", &ChartsWidget::TopTrack::Stats_>,
			};
		}
	}

	ChartsWidget::ChartsWidget (QWidget *parent)
	: QWidget { parent }
	, ChartsView_ { new QQuickWidget }
	, TopArtistsModel_ { MakeSimilarModel (this) }
	, TopTracksModel_ { MakeTracksModel (this) }
	{
		Ui_.setupUi (this);
		layout ()->addWidget (ChartsView_);

		ChartsView_->setResizeMode (QQuickWidget::SizeRootObjectToView);

		ChartsView_->engine ()->addImageProvider (Lits::ThemeIconsUriScheme, new Util::ThemeImageProvider (GetProxyHolder ()));

		new Util::StandardNAMFactory (Lits::LmpSlashQml,
				[] { return 50_mib; },
				ChartsView_->engine ());

		auto objVar = [] (QObject *obj) { return QVariant::fromValue (obj); };
		ChartsView_->rootContext ()->setContextProperties ({
					{ QStringLiteral ("topArtistsModel"), objVar (TopArtistsModel_) },
					{ QStringLiteral ("topTracksModel"), objVar (TopTracksModel_) },
					{ QStringLiteral ("artistsLabelText"), tr ("Top artists") },
					{ QStringLiteral ("tracksLabelText"), tr ("Top tracks") },
					{ QStringLiteral ("colorProxy"), objVar (new Util::ColorThemeProxy (GetProxyHolder ()->GetColorThemeManager (), this)) },
				});

		for (const auto& cand : Util::GetPathCandidates (Util::SysPath::QML, {}))
			ChartsView_->engine ()->addImportPath (cand);

		ChartsView_->setSource (Util::GetSysPathUrl (Util::SysPath::QML, Lits::LmpQmlSubdir, QStringLiteral ("ChartsView.qml")));

		connect (Ui_.InfoProvider_,
				&QComboBox::activated,
				this,
				&ChartsWidget::Request);

		new StdArtistActionsManager { *ChartsView_, this };
	}

	void ChartsWidget::InitializeProviders ()
	{
		const auto& lastProv = XmlSettingsManager::Instance ().Property ("LastUsedChartsProvider", QString {}).toString ();

		bool lastFound = false;

		Providers_ = GetProxyHolder ()->GetPluginsManager ()->GetAllCastableRoots<Media::ITopProvider*> ();
		for (auto provObj : Providers_)
		{
			auto prov = qobject_cast<Media::ITopProvider*> (provObj);

			Ui_.InfoProvider_->addItem (qobject_cast<IInfo*> (provObj)->GetIcon (),
					prov->GetServiceName ());
			if (prov->GetServiceName () == lastProv)
			{
				const int idx = Providers_.size () - 1;
				Ui_.InfoProvider_->setCurrentIndex (idx);
				Request ();
				lastFound = true;
			}
		}

		if (!lastFound)
			Ui_.InfoProvider_->setCurrentIndex (-1);
	}

	void ChartsWidget::Request ()
	{
		TopArtistsModel_->SetItems ({});
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
		auto prov = qobject_cast<Media::ITopProvider*> (provObj);

		Util::Sequence (this, prov->RequestTopArtists ()) >>
				Util::Visitor
				{
					[] (const QString&) { /* TODO */ },
					[this] (const QList<Media::TopArtistInfo>& infos) { HandleArtists (infos); }
				};
		Util::Sequence (this, prov->RequestTopTracks ()) >>
				Util::Visitor
				{
					[] (const QString&) { /* TODO */ },
					[this] (const QList<Media::TopTrackInfo>& infos) { HandleTracks (infos); }
				};

		XmlSettingsManager::Instance ().setProperty ("LastUsedChartsProvider", prov->GetServiceName ());
	}

	namespace
	{
		template<typename T>
		QStringList GetStats (const T& info)
		{
			QStringList stats;
			if (info.Listeners_)
				stats << ChartsWidget::tr ("%n listener(s)", 0, info.Listeners_);
			if (info.Playcount_)
				stats << ChartsWidget::tr ("%n playback(s)", 0, info.Playcount_);
			return stats;
		}
	}

	void ChartsWidget::HandleArtists (const QList<Media::TopArtistInfo>& infos)
	{
		TopArtistsModel_->SetItems (Util::MapAs<QVector> (infos,
				[] (const Media::TopArtistInfo& info)
				{
					SimilarArtistInfo prepared { info.Info_, *Core::Instance ().GetLocalCollection () };
					if (prepared.ShortDesc_.isEmpty ())
						prepared.ShortDesc_ = tr ("%1 is not <em>that</em> mainstream to have a description.")
								.arg (info.Info_.Name_);
					prepared.Similarity_ = GetStats (info).join (u"; ");
					return prepared;
				}));
	}

	void ChartsWidget::HandleTracks (const QList<Media::TopTrackInfo>& infos)
	{
		TopTracksModel_->SetItems (Util::MapAs<QVector> (infos,
				[] (const Media::TopTrackInfo& info) { return TopTrack { info, GetStats (info).join ("; ") }; }));
	}
}
