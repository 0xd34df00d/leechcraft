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
#include <QDeclarativeContext>
#include <QGraphicsObject>
#include <QtDebug>
#include <util/util.h>
#include <interfaces/media/iartistbiofetcher.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "core.h"
#include "biopropproxy.h"

namespace LeechCraft
{
namespace LMP
{
	BioWidget::BioWidget (QWidget *parent)
	: QWidget (parent)
	, BioPropProxy_ (new BioPropProxy (this))
	{
		Ui_.setupUi (this);

		Ui_.View_->rootContext ()->setContextObject (BioPropProxy_);
		Ui_.View_->setSource (QUrl ("qrc:/lmp/resources/qml/BioView.qml"));

		Providers_ = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableTo<Media::IArtistBioFetcher*> ();
		Q_FOREACH (auto provider, Providers_)
			Ui_.Provider_->addItem (provider->GetServiceName ());

		connect (Ui_.Provider_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (requestBiography ()));

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

	void BioWidget::requestBiography ()
	{
		const int idx = Ui_.Provider_->currentIndex ();
		if (idx < 0 || CurrentArtist_.isEmpty ())
			return;

		auto pending = Providers_ [idx]->RequestArtistBio (CurrentArtist_);
		connect (pending->GetObject (),
				SIGNAL (ready ()),
				this,
				SLOT (handleBioReady ()));
	}

	void BioWidget::handleBioReady ()
	{
		auto pending = qobject_cast<Media::IPendingArtistBio*> (sender ());
		const auto& bio = pending->GetArtistBio ();
		BioPropProxy_->SetBio (bio);
	}

	void BioWidget::handleLink (const QString& link)
	{
		Core::Instance ().SendEntity (Util::MakeEntity (QUrl (link),
					QString (),
					FromUserInitiated | OnlyHandle));
	}
}
}
