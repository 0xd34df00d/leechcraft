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

#include "hotstreams.h"
#include <QIcon>
#include <QStandardItem>
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include "somafmlistfetcher.h"
#include "stealkilllistfetcher.h"

#ifdef HAVE_QJSON
#include "audioaddictstreamfetcher.h"
#endif

#include "radiostation.h"
#include "roles.h"

namespace LeechCraft
{
namespace HotStreams
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

#ifdef HAVE_QJSON
		auto di = new QStandardItem ("Digitally Imported");
		di->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		di->setEditable (false);
		di->setIcon (QIcon (":/hotstreams/resources/images/di.png"));
		Roots_ ["di"] = di;

		auto sky = new QStandardItem ("SkyFM");
		sky->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		sky->setEditable (false);
		sky->setIcon (QIcon (":/hotstreams/resources/images/skyfm.png"));
		Roots_ ["sky"] = sky;
#endif

		auto somafm = new QStandardItem ("SomaFM");
		somafm->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		somafm->setEditable (false);
		somafm->setIcon (QIcon (":/hotstreams/resources/images/somafm.png"));
		Roots_ ["somafm"] = somafm;

		auto stealkill = new QStandardItem ("42fm");
		stealkill->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		stealkill->setEditable (false);
		stealkill->setIcon (QIcon (":/hotstreams/resources/images/radio.png"));
		Roots_ ["42fm"] = stealkill;
	}

	void Plugin::SecondInit ()
	{
		QTimer::singleShot (5000,
				this,
				SLOT (refreshRadios ()));
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.HotStreams";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "HotStreams";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides some radio streams like Digitally Imported and SomaFM to other plugins.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QList<QStandardItem*> Plugin::GetRadioListItems () const
	{
		return Roots_.values ();
	}

	Media::IRadioStation_ptr Plugin::GetRadioStation (QStandardItem *item, const QString&)
	{
		const auto& name = item->data (StreamItemRoles::PristineName).toString ();
		const auto& url = item->data (Media::RadioItemRole::RadioID).toUrl ();
		auto nam = Proxy_->GetNetworkAccessManager ();
		const auto& format = item->data (StreamItemRoles::PlaylistFormat).toString ();
		return Media::IRadioStation_ptr (new RadioStation (url, name, nam, format));
	}

	void Plugin::refreshRadios ()
	{
		auto nam = Proxy_->GetNetworkAccessManager ();

		auto clearRoot = [] (QStandardItem *item)
		{
			while (item->rowCount ())
				item->removeRow (0);
		};

		clearRoot (Roots_ ["somafm"]);
		new SomaFMListFetcher (Roots_ ["somafm"], nam, this);

		clearRoot (Roots_ ["42fm"]);
		new StealKillListFetcher (Roots_ ["42fm"], nam, this);

#ifdef HAVE_QJSON
		clearRoot (Roots_ ["di"]);
		new AudioAddictStreamFetcher (AudioAddictStreamFetcher::Service::DI,
				Roots_ ["di"], nam, this);

		clearRoot (Roots_ ["sky"]);
		new AudioAddictStreamFetcher (AudioAddictStreamFetcher::Service::SkyFM,
				Roots_ ["sky"], nam, this);
#endif
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_hotstreams, LeechCraft::HotStreams::Plugin);

