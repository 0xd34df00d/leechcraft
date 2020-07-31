/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "hotstreams.h"
#include <QIcon>
#include <QStandardItem>
#include <QTimer>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <util/sll/delayedexecutor.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "somafmlistfetcher.h"
#include "stealkilllistfetcher.h"
#include "icecastfetcher.h"
#include "icecastmodel.h"
#include "audioaddictstreamfetcher.h"
#include "rockradiolistfetcher.h"
#include "radiostation.h"
#include "roles.h"
#include "stringlistradiostation.h"

Q_DECLARE_METATYPE (QList<QUrl>);

namespace LC
{
namespace HotStreams
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("hotstreams");

		Model_ = new QStandardItemModel;

		Proxy_ = proxy;

		auto nam = Proxy_->GetNetworkAccessManager ();

		auto di = new QStandardItem ("Digitally Imported");
		di->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		di->setEditable (false);
		di->setIcon (QIcon (":/hotstreams/resources/images/di.png"));
		Root2Fetcher_ [di] = [nam, this] (QStandardItem *di)
			{
				new AudioAddictStreamFetcher (AudioAddictStreamFetcher::Service::DI,
						di, nam, this);
			};
		Model_->appendRow (di);

		auto sky = new QStandardItem ("SkyFM");
		sky->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		sky->setEditable (false);
		sky->setIcon (QIcon (":/hotstreams/resources/images/skyfm.png"));
		Root2Fetcher_ [sky] = [nam, this] (QStandardItem *sky)
			{
				new AudioAddictStreamFetcher (AudioAddictStreamFetcher::Service::SkyFM,
						sky, nam, this);
			};
		Model_->appendRow (sky);

		auto rr = new QStandardItem ("RockRadio");
		rr->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		rr->setEditable (false);
		rr->setIcon (QIcon (":/hotstreams/resources/images/rockradio.png"));
		Root2Fetcher_ [rr] = [nam, this] (QStandardItem *rr)
				{ new RockRadioListFetcher (rr, nam, this); };
		Model_->appendRow (rr);

		auto somafm = new QStandardItem ("SomaFM");
		somafm->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		somafm->setEditable (false);
		somafm->setIcon (QIcon (":/hotstreams/resources/images/somafm.png"));
		Root2Fetcher_ [somafm] = [nam, this] (QStandardItem *somafm)
				{ new SomaFMListFetcher (somafm, nam, this); };
		Model_->appendRow (somafm);

		auto stealkill = new QStandardItem ("42fm");
		stealkill->setData (Media::RadioType::None, Media::RadioItemRole::ItemType);
		stealkill->setEditable (false);
		stealkill->setIcon (QIcon (":/hotstreams/resources/images/radio.png"));
		Root2Fetcher_ [stealkill] = [nam, this] (QStandardItem *stealkill)
				{ new StealKillListFetcher (stealkill, nam, this); };
		Model_->appendRow (stealkill);

		IcecastModel_ = new IcecastModel;
		Model2Fetcher_ [IcecastModel_] = [proxy, this] { new IcecastFetcher (IcecastModel_, proxy, this); };
	}

	void Plugin::SecondInit ()
	{
		Util::ExecuteLater ([this] { RefreshItems ({}); }, 5000);
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
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QList<QAbstractItemModel*> Plugin::GetRadioListItems () const
	{
		return { Model_, IcecastModel_ };
	}

	Media::IRadioStation_ptr Plugin::GetRadioStation (const QModelIndex& index, const QString&)
	{
		const auto& name = index.data (StreamItemRoles::PristineName).toString ();
		const auto& format = index.data (StreamItemRoles::PlaylistFormat).toString ();
		if (format != "urllist")
		{
			auto nam = Proxy_->GetNetworkAccessManager ();
			const auto& url = index.data (Media::RadioItemRole::RadioID).toUrl ();
			return std::make_shared<RadioStation> (url, name, nam, format);
		}
		else
		{
			const auto& urlList = index.data (StreamItemRoles::UrlList).value<QList<QUrl>> ();
			return std::make_shared<StringListRadioStation> (urlList, name);
		}
	}

	void Plugin::RefreshItems (const QList<QModelIndex>& indices)
	{
		auto clearRoot = [] (QStandardItem *item)
		{
			if (const auto rc = item->rowCount ())
				item->removeRows (0, rc);
		};

		const auto& items = indices.isEmpty () ?
				Root2Fetcher_.keys () :
				Util::Map (indices,
						[this] (const QModelIndex& index) { return Model_->itemFromIndex (index); });
		for (auto item : items)
		{
			if (!Root2Fetcher_.contains (item))
				continue;

			clearRoot (item);
			Root2Fetcher_ [item] (item);
		}

		auto models = indices.isEmpty () ?
				Model2Fetcher_.keys () :
				Util::Map (indices,
						[] (const QModelIndex& index) { return index.model (); });
		models.removeAll (Model_);
		for (auto model : models)
			Model2Fetcher_ [model] ();
	}
}
}

LC_EXPORT_PLUGIN (leechcraft_hotstreams, LC::HotStreams::Plugin);

