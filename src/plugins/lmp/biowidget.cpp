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
#include <QtDebug>
#include <util/util.h>
#include <interfaces/media/iartistbiofetcher.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "xmlsettingsmanager.h"
#include "core.h"
#include "bioviewmanager.h"
#include "previewhandler.h"

namespace LeechCraft
{
namespace LMP
{
	BioWidget::BioWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		Manager_ = new BioViewManager (Ui_.View_, this);
		Ui_.View_->setSource (QUrl ("qrc:/lmp/resources/qml/BioView.qml"));
		Manager_->InitWithSource ();

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

		connect (Manager_,
				SIGNAL (gotArtistImage (QString, QUrl)),
				this,
				SIGNAL (gotArtistImage (QString, QUrl)));
	}

	void BioWidget::SetCurrentArtist (const QString& artist)
	{
		if (artist == CurrentArtist_)
			return;

		CurrentArtist_ = artist;
		requestBiography ();
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
		const int idx = Ui_.Provider_->currentIndex ();
		if (idx < 0 || CurrentArtist_.isEmpty ())
			return;

		Manager_->Request (Providers_ [idx], CurrentArtist_);
	}
}
}
