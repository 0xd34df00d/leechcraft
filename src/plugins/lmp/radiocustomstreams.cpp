/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "radiocustomstreams.h"
#include <QStandardItemModel>
#include <QtDebug>
#include <interfaces/media/iradiostation.h>
#include <interfaces/media/audiostructs.h>
#include <interfaces/core/iiconthememanager.h>
#include "radiocustomstation.h"
#include "xmlsettingsmanager.h"
#include "core.h"

typedef QList<QPair<QString, QUrl>> CustomStationsList_t;
Q_DECLARE_METATYPE (CustomStationsList_t);

namespace LC
{
namespace LMP
{
	namespace
	{
		enum CustomRole
		{
			UrlRole = Media::RadioItemRole::MaxRadioRole + 1
		};
	}

	RadioCustomStreams::RadioCustomStreams (QObject *parent)
	: QObject { parent }
	, Model_ { new QStandardItemModel { this } }
	, Root_ { new QStandardItem { tr ("Custom streams") } }
	{
		Root_->setIcon (Core::Instance ().GetProxy ()->
					GetIconThemeManager ()->GetIcon ("favorites"));
		Root_->setData (Media::RadioType::CustomAddableStreams, Media::RadioItemRole::ItemType);
		Root_->setData ("org.LeechCraft.LMP.Custom", Media::RadioItemRole::RadioID);
		Root_->setEditable (false);
		Model_->appendRow (Root_);

		LoadSettings ();
	}

	QList<QAbstractItemModel*> RadioCustomStreams::GetRadioListItems () const
	{
		return { Model_ };
	}

	Media::IRadioStation_ptr RadioCustomStreams::GetRadioStation (const QModelIndex& item, const QString&)
	{
		QList<QUrl> urls;
		if (item == Root_->index ())
			urls = GetAllUrls ();
		else
			urls << item.data (CustomRole::UrlRole).toUrl ();
		return std::make_shared<RadioCustomStation> (urls, this);
	}

	void RadioCustomStreams::RefreshItems (const QList<QModelIndex>&)
	{
	}

	void RadioCustomStreams::Add (const QUrl& url, const QString& name)
	{
		if (GetAllUrls ().contains (url))
			return;

		CreateItem (url, name);

		SaveSettings ();
	}

	void RadioCustomStreams::Remove (const QModelIndex& index)
	{
		for (auto i = 0; i < Root_->rowCount (); ++i)
			if (Root_->child (i)->index () == index)
			{
				Root_->removeRow (i);
				SaveSettings ();
				break;
			}
	}

	void RadioCustomStreams::CreateItem (const QUrl& url, const QString& name)
	{
		const auto& urlStr = url.toString ();
		auto item = new QStandardItem (name.isEmpty () ? urlStr : name);
		item->setEditable (false);

		item->setToolTip (urlStr);
		item->setData (url, CustomRole::UrlRole);
		item->setData (Media::RadioType::SingleTrack, Media::RadioItemRole::ItemType);

		Media::AudioInfo info;
		info.Length_ = 0;
		info.Other_ ["URL"] = url;
		item->setData (QVariant::fromValue (QList<Media::AudioInfo> { info }),
				Media::RadioItemRole::TracksInfos);

		Root_->appendRow (item);
	}

	QList<QUrl> RadioCustomStreams::GetAllUrls () const
	{
		QList<QUrl> result;
		for (auto i = 0; i < Root_->rowCount (); ++i)
			result << Root_->child (i)->data (CustomRole::UrlRole).toUrl ();
		return result;
	}

	void RadioCustomStreams::LoadSettings ()
	{
		const auto& pairs = XmlSettingsManager::Instance ()
				.property ("CustomRadioUrls").value<CustomStationsList_t> ();
		for (const auto& pair : pairs)
			CreateItem (pair.second, pair.first);
	}

	void RadioCustomStreams::SaveSettings () const
	{
		CustomStationsList_t list;
		for (auto i = 0; i < Root_->rowCount (); ++i)
		{
			const auto item = Root_->child (i);
			list.append ({ item->text (), item->data (CustomRole::UrlRole).toUrl () });
		}
		XmlSettingsManager::Instance ()
				.setProperty ("CustomRadioUrls", QVariant::fromValue (list));
	}
}
}
