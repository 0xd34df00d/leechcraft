/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
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

#include "lastfmscrobble.h"
#include <QIcon>
#include <QByteArray>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/entitytesthandleresult.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/passutils.h>
#include "lastfmsubmitter.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Lastfmscrobble
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"lastfmscrobblesettings.xml");

		LFSubmitter_ = new LastFMSubmitter (this);

		XmlSettingsManager::Instance ().RegisterObject ("lastfm.login",
				this, "handleSubmitterInit");
		handleSubmitterInit ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Lastfmscrobble";
	}

	QString Plugin::GetName () const
	{
		return "Last.FM Scrobbler";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Submits information about tracks you've listened to Last.FM.");
	}

	void Plugin::Release ()
	{
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon (":/resources/images/lastfmscrobble.svg");
	}

	EntityTestHandleResult Plugin::CouldHandle (const Entity& entity) const
	{
		return entity.Mime_ == "x-leechcraft/now-playing-track-info" ?
				EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
				EntityTestHandleResult ();
	}

	void Plugin::Handle (Entity entity)
	{
		LFSubmitter_->submit ();

		MediaMeta meta (entity.Additional_);

		LFSubmitter_->sendTrack (meta);
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	void Plugin::handleSubmitterInit ()
	{
		const QString& login = XmlSettingsManager::Instance ()
				.property ("lastfm.login").toString ();
		LFSubmitter_->SetUsername (login);

		QString password;
		if (!login.isEmpty ())
		{
			password = Util::GetPassword ("org.LeechCraft.Lastfmscrobble/" + login,
					tr ("Enter password for Last.fm account with login %1:")
						.arg (login),
					this);
			return;
		}

		LFSubmitter_->SetPassword (password);
		LFSubmitter_->Init (Proxy_->GetNetworkAccessManager ());
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_lastfmscrobble, LeechCraft::Lastfmscrobble::Plugin);
