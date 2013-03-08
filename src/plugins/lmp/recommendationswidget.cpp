/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#include "recommendationswidget.h"
#include <QtDebug>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/irecommendedartists.h>
#include <interfaces/media/iaudioscrobbler.h>
#include <interfaces/media/ipendingsimilarartists.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "util.h"

namespace LeechCraft
{
namespace LMP
{
	RecommendationsWidget::RecommendationsWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
	}

	void RecommendationsWidget::InitializeProviders ()
	{
		const auto& lastProv = ShouldRememberProvs () ?
				XmlSettingsManager::Instance ()
					.Property ("LastUsedRecsProvider", QString ()).toString () :
				QString ();

		bool lastFound = false;

		const auto& roots = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableRoots<Media::IRecommendedArtists*> ();
		Q_FOREACH (auto root, roots)
		{
			auto scrob = qobject_cast<Media::IAudioScrobbler*> (root);
			if (!scrob)
				continue;

			Ui_.RecProvider_->addItem (scrob->GetServiceName ());
			ProvRoots_ << root;
			Providers_ << qobject_cast<Media::IRecommendedArtists*> (root);

			if (scrob->GetServiceName () == lastProv)
			{
				const int idx = Providers_.size () - 1;
				Ui_.RecProvider_->setCurrentIndex (idx);
				on_RecProvider__activated (idx);
				lastFound = true;
			}
		}

		if (!lastFound)
			Ui_.RecProvider_->setCurrentIndex (-1);
	}

	void RecommendationsWidget::handleGotRecs ()
	{
		auto pending = qobject_cast<Media::IPendingSimilarArtists*> (sender ());
		if (!pending)
		{
			qWarning () << Q_FUNC_INFO
					<< "not a pending sender"
					<< sender ();
			return;
		}
		const auto& similars = pending->GetSimilar ();

		Ui_.RecView_->SetSimilarArtists (similars);
	}

	void RecommendationsWidget::on_RecProvider__activated (int index)
	{
		if (index < 0 || index >= Providers_.size ())
			return;

		auto pending = Providers_.at (index)->RequestRecommended (10);
		connect (pending->GetObject (),
				SIGNAL (ready ()),
				this,
				SLOT (handleGotRecs ()));

		auto scrob = qobject_cast<Media::IAudioScrobbler*> (ProvRoots_.at (index));
		XmlSettingsManager::Instance ()
				.setProperty ("LastUsedRecsProvider", scrob->GetServiceName ());
	}
}
}
