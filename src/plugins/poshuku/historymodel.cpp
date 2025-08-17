/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "historymodel.h"
#include <memory>
#include <QTimer>
#include <QVariant>
#include <QAction>
#include <QtDebug>
#include <util/xpc/defaulthookproxy.h>
#include <util/sll/prelude.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "poshuku.h"

namespace LC
{
namespace Poshuku
{
	namespace
	{
		/** Returns the number of the section for the given date.
			*
			* - Today
			* - Yesterday
			* - Two days ago
			* - Last week
			* - Last month
			* - Last 2 months
			* - Last 3 months
			* - Last 4 months
			* - ...
			* - Last N months
			*/
		int SectionNumber (const QDateTime& date, QDateTime current = QDateTime ())
		{
			if (!current.isValid ())
				current = QDateTime::currentDateTime ();

			QDate orig = current.date ();
			if (date.daysTo (current) == 0)
				return 0;
			if (date.daysTo (current) == 1)
				return 1;
			if (date.daysTo (current) == 2)
				return 2;
			if (date.daysTo (current) <= 7)
				return 3;

			int i = 0;
			while (true)
			{
				current.setDate (orig.addMonths (--i));

				if (date.daysTo (current) <= 0)
					return -i + 3;
			}
		}

		QString SectionName (int number)
		{
			switch (number)
			{
			case 0:
				return QObject::tr ("Today");
			case 1:
				return QObject::tr ("Yesterday");
			case 2:
				return QObject::tr ("Two days ago");
			case 3:
				return QObject::tr ("Last week");
			case 4:
				return QObject::tr ("Last month");
			default:
				return QObject::tr ("Last %n month(s)", "", number - 3);
			}
		}
	};

	HistoryModel::HistoryModel (QObject *parent)
	: QStandardItemModel { parent }
	, GarbageTimer_ { new QTimer { this } }
	{
		setHorizontalHeaderLabels ({ tr ("Title"), tr ("URL"), tr ("Date") });
	}

	void HistoryModel::HandleStorageReady ()
	{
		loadData ();

		GarbageTimer_->start (15 * 60 * 1000);
		connect (GarbageTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (collectGarbage ()));
	}

	void HistoryModel::addItem (QString title, QString url, QDateTime date)
	{
		if (title.isEmpty () || url.isEmpty ())
			return;

		if (url == "about:blank" || url.startsWith ("lc://"))
			return;

		auto proxy = std::make_shared<Util::DefaultHookProxy> ();
		emit hookAddingToHistory (proxy, title, url, date);
		if (proxy->IsCancelled ())
			return;

		QVariantList result = proxy->GetReturnValue ().toList ();
		int size = result.size ();
		if (size >= 1)
			title = result.at (0).toString ();
		if (size >= 2)
			url = result.at (1).toString ();
		if (size >= 3)
			date = result.at (2).toDateTime ();

		HistoryItem item =
		{
			title,
			date,
			url
		};
		Core::Instance ().GetStorageBackend ()->AddToHistory (item);
	}

	QList<QMap<QString, QVariant>> HistoryModel::getItemsMap () const
	{
		return Util::Map (Items_,
				[] (const auto& item)
				{
					return QVariantMap
					{
						{ "Title", item.Title_ },
						{ "DateTime", item.DateTime_ },
						{ "URL", item.URL_ }
					};
				});
	}

	void HistoryModel::Add (const HistoryItem& histItem, int section)
	{
		while (section >= rowCount ())
		{
			const auto& folderIcon = Core::Instance ().GetProxy ()->
					GetIconThemeManager ()->GetIcon ("document-open-folder");

			const QList<QStandardItem*> sectItems
			{
				new QStandardItem { folderIcon, SectionName (rowCount ()) },
				new QStandardItem,
				new QStandardItem
			};
			for (const auto item : sectItems)
				item->setEditable (false);

			appendRow (sectItems);
		}

		const auto icon = Core::Instance ().GetIcon (QUrl { histItem.URL_ });
		auto normalizeText = [] (QString text)
		{
			return text.trimmed ().replace ('\n', ' ');
		};
		const QList<QStandardItem*> items
		{
			new QStandardItem { icon, normalizeText (histItem.Title_) },
			new QStandardItem { normalizeText (histItem.URL_) },
			new QStandardItem { QLocale {}.toString (histItem.DateTime_, QLocale::ShortFormat) }
		};
		for (const auto item : items)
			item->setEditable (false);
		item (section)->appendRow (items);
	}

	void HistoryModel::loadData ()
	{
		collectGarbage ();

		if (const auto rc = rowCount ())
			removeRows (0, rc);

		Items_.clear ();
		Core::Instance ().GetStorageBackend ()->LoadHistory (Items_);

		QSet<QString> urls;
		for (auto i = Items_.begin (); i != Items_.end (); )
		{
			if (urls.contains (i->URL_))
				i = Items_.erase (i);
			else
			{
				urls << i->URL_;
				++i;
			}
		}

		const auto& now = QDateTime::currentDateTime ();
		for (const auto& item : Items_)
			Add (item, SectionNumber (item.DateTime_, now));
	}

	void HistoryModel::handleItemAdded (const HistoryItem& item)
	{
		Items_.push_back (item);
		Add (item, SectionNumber (item.DateTime_));
	}

	void HistoryModel::collectGarbage ()
	{
		int age = XmlSettingsManager::Instance ().property ("HistoryClearOlderThan").toInt ();
		int maxItems = XmlSettingsManager::Instance ().property ("HistoryKeepLessThan").toInt ();
		Core::Instance ().GetStorageBackend ()->ClearOldHistory (age, maxItems);
	}
}
}
