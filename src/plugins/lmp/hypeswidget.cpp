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

#include "hypeswidget.h"
#include <QStandardItemModel>
#include <QDeclarativeContext>
#include <interfaces/media/ihypesprovider.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "util.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "similarmodel.h"

namespace LeechCraft
{
namespace LMP
{
	HypesWidget::HypesWidget (QWidget *parent)
	: QWidget (parent)
	, ArtistsModel_ (new SimilarModel (this))
	{
		Ui_.setupUi (this);
		Ui_.HypesView_->rootContext ()->setContextProperty ("artistsModel", ArtistsModel_);
		Ui_.HypesView_->setSource (QUrl ("qrc:/lmp/resources/qml/HypesView.qml"));

		connect (Ui_.InfoProvider_,
				SIGNAL (activated (int)),
				this,
				SLOT (request ()));
	}

	void HypesWidget::InitializeProviders ()
	{
		const auto& lastProv = ShouldRememberProvs () ?
				XmlSettingsManager::Instance ()
					.Property ("LastUsedHypesProvider", QString ()).toString () :
				QString ();

		bool lastFound = false;

		Providers_ = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableTo<Media::IHypesProvider*> ();
		for (auto prov : Providers_)
		{
			Ui_.InfoProvider_->addItem (prov->GetServiceName ());

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

	void HypesWidget::request ()
	{
		ArtistsModel_->clear ();

		const auto idx = Ui_.InfoProvider_->currentIndex ();
		if (idx < 0)
			return;

		for (auto prov : Providers_)
			disconnect (dynamic_cast<QObject*> (prov),
					0,
					this,
					0);

		auto prov = Providers_.at (idx);
		if (prov->SupportsHype (Media::IHypesProvider::HypeType::Artist))
		{
			connect (dynamic_cast<QObject*> (prov),
					SIGNAL (gotHypedArtists (QList<Media::HypedArtistInfo>)),
					this,
					SLOT (handleArtists (QList<Media::HypedArtistInfo>)));
			prov->RequestHype (Media::IHypesProvider::HypeType::Artist);
		}

		XmlSettingsManager::Instance ()
				.setProperty ("LastUsedReleasesProvider", prov->GetServiceName ());
	}

	void HypesWidget::handleArtists (const QList<Media::HypedArtistInfo>& infos)
	{
		for (const auto& info : infos)
		{
			auto item = new QStandardItem;

			const auto& artist = info.Info_;
			item->setData (artist.Name_, SimilarModel::Role::ArtistName);
			item->setData (artist.Image_, SimilarModel::Role::ArtistImageURL);
			item->setData (artist.LargeImage_, SimilarModel::Role::ArtistBigImageURL);
			item->setData (artist.ShortDesc_, SimilarModel::Role::ShortDesc);
			item->setData (artist.FullDesc_, SimilarModel::Role::FullDesc);
			item->setData (artist.Page_, SimilarModel::Role::ArtistPageURL);

			const auto& perc = tr ("Percentage change: %1%").arg (info.PercentageChange_);
			item->setData (perc, SimilarModel::Role::Similarity);

			ArtistsModel_->appendRow (item);
		}

		disconnect (sender (),
				SIGNAL (gotHypedArtists (QList<Media::HypedArtistInfo>)),
				this,
				SLOT (handleArtists (QList<Media::HypedArtistInfo>)));
	}
}
}
