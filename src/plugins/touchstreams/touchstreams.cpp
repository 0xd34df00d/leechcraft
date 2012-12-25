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

#include "touchstreams.h"
#include <QIcon>
#include <util/queuemanager.h>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "authmanager.h"
#include "audiosearch.h"

namespace LeechCraft
{
namespace TouchStreams
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		Util::InstallTranslator ("touchstreams");

		Queue_ = new Util::QueueManager (350);

		AuthMgr_ = new AuthManager (proxy);

		XSD_.reset (new Util::XmlSettingsDialog);
		XSD_->RegisterObject (&XmlSettingsManager::Instance (), "touchstreamssettings.xml");

		connect (XSD_.get (),
				SIGNAL (pushButtonClicked (QString)),
				this,
				SLOT (handlePushButton (QString)));
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.TouchStreams";
	}

	void Plugin::Release ()
	{
		delete Queue_;
	}

	QString Plugin::GetName () const
	{
		return "TouchStreams";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("VK.com music streamer.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XSD_;
	}

	Media::IPendingAudioSearch* Plugin::Search (const Media::AudioSearchRequest& req)
	{
		auto realReq = req;
		if (realReq.FreeForm_.isEmpty ())
		{
			QStringList parts = { req.Artist_, req.Album_, req.Title_ };
			parts.removeAll (QString ());
			realReq.FreeForm_ = parts.join (" - ");
		}

		return new AudioSearch (Proxy_, realReq, AuthMgr_, Queue_);
	}

	void Plugin::handlePushButton (const QString& name)
	{
		if (name == "AllowRequestsTriggered")
			AuthMgr_->Reauth ();
		else
			qWarning () << Q_FUNC_INFO
					<< "unknown name"
					<< name;
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_touchstreams, LeechCraft::TouchStreams::Plugin);
