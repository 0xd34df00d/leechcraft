/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "biowidget.h"
#include <QStandardItemModel>
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/media/iartistbiofetcher.h>
#include <interfaces/media/idiscographyprovider.h>
#include <interfaces/media/ialbumartprovider.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "core.h"
#include "biopropproxy.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace LMP
{
	namespace
	{
		class DiscoModel : public QStandardItemModel
		{
		public:
			enum Roles
			{
				AlbumName = Qt::UserRole + 1,
				AlbumYear,
				AlbumImage
			};

			DiscoModel (QObject *parent)
			: QStandardItemModel (parent)
			{
				QHash<int, QByteArray> roleNames;
				roleNames [Roles::AlbumName] = "albumName";
				roleNames [Roles::AlbumYear] = "albumYear";
				roleNames [Roles::AlbumImage] = "albumImage";
				setRoleNames (roleNames);
			}
		};

		const int AASize = 170;
	}

	BioWidget::BioWidget (QWidget *parent)
	: QWidget (parent)
	, BioPropProxy_ (new BioPropProxy (this))
	, DiscoModel_ (new DiscoModel (this))
	{
		Ui_.setupUi (this);

		Ui_.View_->rootContext ()->setContextObject (BioPropProxy_);
		Ui_.View_->rootContext ()->setContextProperty ("artistDiscoModel", DiscoModel_);
		Ui_.View_->setSource (QUrl ("qrc:/lmp/resources/qml/BioView.qml"));

		const auto& lastProv = XmlSettingsManager::Instance ()
				.Property ("LastUsedBioProvider", QString ()).toString ();

		Providers_ = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableTo<Media::IArtistBioFetcher*> ();
		Q_FOREACH (auto provider, Providers_)
		{
			Ui_.Provider_->addItem (provider->GetServiceName ());
			if (lastProv == provider->GetServiceName ())
				Ui_.Provider_->setCurrentIndex (Ui_.Provider_->count () - 1);
		}

		connect (Ui_.Provider_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (requestBiography ()));
		connect (Ui_.Provider_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (saveLastUsedProv ()));

		connect (Ui_.View_->rootObject (),
				SIGNAL (linkActivated (QString)),
				this,
				SLOT (handleLink (QString)));
	}

	void BioWidget::SetCurrentArtist (const QString& artist)
	{
		if (artist == CurrentArtist_)
			return;

		CurrentArtist_ = artist;
		requestBiography ();
	}

	QStandardItem* BioWidget::FindAlbumItem (const QString& albumName) const
	{
		for (int i = 0, rc = DiscoModel_->rowCount (); i < rc; ++i)
		{
			auto item = DiscoModel_->item (i);
			const auto& itemData = item->data (DiscoModel::Roles::AlbumName);
			if (itemData.toString () == albumName)
				return item;
		}
		return 0;
	}

	void BioWidget::saveLastUsedProv ()
	{
		const int idx = Ui_.Provider_->currentIndex ();
		const auto& prov = idx >= 0 ?
				Providers_.value (idx)->GetServiceName () :
				QString ();

		XmlSettingsManager::Instance ().setProperty ("LastUsedBioProvider", prov);
	}

	void BioWidget::requestBiography ()
	{
		DiscoModel_->clear ();

		const int idx = Ui_.Provider_->currentIndex ();
		if (idx < 0 || CurrentArtist_.isEmpty ())
			return;

		auto pending = Providers_ [idx]->RequestArtistBio (CurrentArtist_);
		connect (pending->GetObject (),
				SIGNAL (ready ()),
				this,
				SLOT (handleBioReady ()));

		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		for (auto prov : pm->GetAllCastableTo<Media::IDiscographyProvider*> ())
		{
			auto fetcher = prov->GetDiscography (CurrentArtist_);
			connect (fetcher->GetObject (),
					SIGNAL (ready ()),
					this,
					SLOT (handleDiscographyReady ()));
		}
	}

	void BioWidget::handleBioReady ()
	{
		auto pending = qobject_cast<Media::IPendingArtistBio*> (sender ());
		const auto& bio = pending->GetArtistBio ();
		BioPropProxy_->SetBio (bio);

		emit gotArtistImage (bio.BasicInfo_.Name_, bio.BasicInfo_.LargeImage_);
	}

	void BioWidget::handleDiscographyReady ()
	{
		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		auto aaProvObj = pm->GetAllCastableRoots<Media::IAlbumArtProvider*> ().value (0);
		auto aaProv = qobject_cast<Media::IAlbumArtProvider*> (aaProvObj);
		if (aaProvObj)
			connect (aaProvObj,
					SIGNAL (gotAlbumArt (Media::AlbumInfo, QList<QImage>)),
					this,
					SLOT (handleAlbumArt (Media::AlbumInfo, QList<QImage>)),
					Qt::UniqueConnection);

		auto fetcher = qobject_cast<Media::IPendingDisco*> (sender ());
		for (const auto& release : fetcher->GetReleases ())
		{
			if (FindAlbumItem (release.Name_))
				continue;

			auto item = new QStandardItem;
			item->setData (release.Name_, DiscoModel::Roles::AlbumName);
			item->setData (QString::number (release.Year_), DiscoModel::Roles::AlbumYear);
			DiscoModel_->appendRow (item);

			aaProv->RequestAlbumArt ({ CurrentArtist_, release.Name_ });
		}
	}

	void BioWidget::handleAlbumArt (const Media::AlbumInfo& info, const QList<QImage>& images)
	{
		if (info.Artist_ != CurrentArtist_ || images.isEmpty ())
			return;

		auto item = FindAlbumItem (info.Album_);
		if (!item)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown item for"
					<< info.Album_;
			return;
		}

		auto img = images.first ();
		if (img.width () > AASize)
			img = img.scaled (AASize, AASize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
		item->setData (Util::GetAsBase64Src (img), DiscoModel::Roles::AlbumImage);
	}

	void BioWidget::handleLink (const QString& link)
	{
		Core::Instance ().SendEntity (Util::MakeEntity (QUrl (link),
					QString (),
					FromUserInitiated | OnlyHandle));
	}
}
}
