/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
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

#include "musiczombie.h"
#include <QIcon>
#include <interfaces/core/icoreproxy.h>
#include <util/queuemanager.h>
#include "pendingdisco.h"

namespace LeechCraft
{
namespace MusicZombie
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Queue_ = new Util::QueueManager (500);
		Proxy_ = proxy;
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.MusicZombie";
	}

	void Plugin::Release ()
	{
		delete Queue_;
	}

	QString Plugin::GetName () const
	{
		return "MusicZombie";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Client for the MusicBrainz.org service.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QString Plugin::GetServiceName () const
	{
		return "MusicBrainz.org";
	}

	Media::IPendingDisco* Plugin::GetDiscography (const QString& artist)
	{
		return new PendingDisco (Queue_, artist, QString (),
				Proxy_->GetNetworkAccessManager (), this);
	}

	Media::IPendingDisco* Plugin::GetReleaseInfo (const QString& artist, const QString& release)
	{
		return new PendingDisco (Queue_, artist, release,
				Proxy_->GetNetworkAccessManager (), this);
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_musiczombie, LeechCraft::MusicZombie::Plugin);
