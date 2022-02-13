/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "releaseswidget.h"
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QStandardItemModel>
#include <QtDebug>
#include <util/xpc/util.h>
#include <util/qml/colorthemeproxy.h>
#include <util/qml/standardnamfactory.h>
#include <util/qml/themeimageprovider.h>
#include <util/sys/paths.h>
#include <util/sll/visitor.h>
#include <util/threads/futures.h>
#include <util/models/rolenamesmixin.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/irecentreleases.h>
#include <interfaces/media/idiscographyprovider.h>
#include <interfaces/iinfo.h>
#include "literals.h"
#include "stdartistactionsmanager.h"
#include "xmlsettingsmanager.h"
#include "util.h"

namespace LC
{
namespace LMP
{
	namespace
	{
		class ReleasesModel : public Util::RoleNamesMixin<QStandardItemModel>
		{
		public:
			enum Role
			{
				AlbumName = Qt::UserRole + 1,
				ArtistName,
				AlbumImageThumb,
				AlbumImageFull,
				ReleaseDate,
				ReleaseURL,
				TrackList
			};

			ReleasesModel (QObject *parent = 0)
			: RoleNamesMixin<QStandardItemModel> (parent)
			{
				QHash<int, QByteArray> names;
				names [AlbumName] = "albumName";
				names [ArtistName] = "artistName";
				names [AlbumImageThumb] = "albumThumbImage";
				names [AlbumImageFull] = "albumFullImage";
				names [ReleaseDate] = "releaseDate";
				names [ReleaseURL] = "releaseURL";
				names [TrackList] = "trackList";
				setRoleNames (names);
			}
		};
	}

	ReleasesWidget::ReleasesWidget (QWidget *parent)
	: QWidget (parent)
	, ReleasesView_ (new QQuickWidget)
	, ReleasesModel_ (new ReleasesModel (this))
	{
		Ui_.setupUi (this);
		layout ()->addWidget (ReleasesView_);

		ReleasesView_->setResizeMode (QQuickWidget::SizeRootObjectToView);

		new Util::StandardNAMFactory ("lmp/qml",
				[] { return 50 * 1024 * 1024; },
				ReleasesView_->engine ());

		ReleasesView_->engine ()->addImageProvider (Lits::ThemeIconsUriScheme, new Util::ThemeImageProvider (GetProxyHolder ()));
		ReleasesView_->rootContext ()->setContextProperty ("releasesModel", ReleasesModel_);
		ReleasesView_->rootContext ()->setContextProperty ("colorProxy",
				new Util::ColorThemeProxy (GetProxyHolder ()->GetColorThemeManager (), this));
		ReleasesView_->setSource (Util::GetSysPathUrl (Util::SysPath::QML, "lmp", "ReleasesView.qml"));

		connect (Ui_.InfoProvider_,
				SIGNAL (activated (int)),
				this,
				SLOT (request ()));
		connect (Ui_.WithRecs_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (request ()));

		new StdArtistActionsManager { *ReleasesView_, this };
	}

	void ReleasesWidget::InitializeProviders ()
	{
		auto pm = GetProxyHolder ()->GetPluginsManager ();

		const auto& lastProv = XmlSettingsManager::Instance ().Property ("LastUsedReleasesProvider", QString ()).toString ();

		bool lastFound = false;

		const auto& objList = pm->GetAllCastableRoots<Media::IRecentReleases*> ();
		for (auto provObj : objList)
		{
			const auto prov = qobject_cast<Media::IRecentReleases*> (provObj);
			Providers_ << prov;

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

		DiscoProviders_ = pm->GetAllCastableTo<Media::IDiscographyProvider*> ();
	}

	void ReleasesWidget::HandleRecentReleases (const QList<Media::AlbumRelease>& releases)
	{
		TrackLists_.resize (releases.size ());

		auto discoProv = DiscoProviders_.value (0);
		QLocale loc;
		for (const auto& release : releases)
		{
			auto item = new QStandardItem ();
			item->setData (release.Title_, ReleasesModel::Role::AlbumName);
			item->setData (release.Artist_, ReleasesModel::Role::ArtistName);
			item->setData (release.ThumbImage_, ReleasesModel::Role::AlbumImageThumb);
			item->setData (release.FullImage_, ReleasesModel::Role::AlbumImageFull);
			item->setData (loc.toString (release.Date_.date (), QLocale::LongFormat), ReleasesModel::Role::ReleaseDate);
			item->setData (release.ReleaseURL_, ReleasesModel::Role::ReleaseURL);
			item->setData (QString (), ReleasesModel::Role::TrackList);
			ReleasesModel_->appendRow (item);

			if (discoProv)
				Util::Sequence (this, discoProv->GetReleaseInfo (release.Artist_, release.Title_)) >>
						Util::Visitor
						{
							[] (const QString&) { qWarning () << Q_FUNC_INFO << "error fetching releases"; },
							[=, this, row = ReleasesModel_->rowCount ()] (const auto& infos)
							{
								if (infos.isEmpty ())
									return;

								if (row >= ReleasesModel_->rowCount () || ReleasesModel_->item (row) != item)
								{
									qWarning () << Q_FUNC_INFO
											<< "model has been invalidated";
									return;
								}

								item->setData (MakeTrackListTooltip (infos [0].TrackInfos_),
										ReleasesModel::Role::TrackList);
							}
						};
		}
	}

	void ReleasesWidget::request ()
	{
		TrackLists_.clear ();
		ReleasesModel_->clear ();

		const auto idx = Ui_.InfoProvider_->currentIndex ();
		if (idx < 0)
			return;

		const bool withRecs = Ui_.WithRecs_->checkState () == Qt::Checked;
		auto prov = Providers_.at (idx);
		Util::Sequence (this, prov->RequestRecentReleases (15, withRecs)) >>
				Util::Visitor
				{
					[] (const QString&) { /* TODO */ },
					[this] (const QList<Media::AlbumRelease>& releases) { HandleRecentReleases (releases); }
				};

		XmlSettingsManager::Instance ().setProperty ("LastUsedReleasesProvider", prov->GetServiceName ());
	}
}
}
