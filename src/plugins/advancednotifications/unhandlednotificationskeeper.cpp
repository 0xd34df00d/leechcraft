/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "unhandlednotificationskeeper.h"
#include <QStandardItemModel>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/an/ianemitter.h>
#include <util/xpc/anutil.h>
#include <util/xpc/stdanfields.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>

namespace LC::AdvancedNotifications
{
	UnhandledNotificationsKeeper::UnhandledNotificationsKeeper (QObject *parent)
	: QObject { parent }
	, Model_ { new QStandardItemModel { this } }
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Title"), tr ("Text"), tr ("Category"), tr ("Type") });
	}

	void UnhandledNotificationsKeeper::AddUnhandled (const Entity& e)
	{
		const int maxCount = 1000;
		while (Model_->rowCount () >= maxCount)
			Model_->removeRow (0);

		const auto& category = e.Additional_ [AN::EF::EventCategory].toString ();
		const auto& type = e.Additional_ [AN::EF::EventType].toString ();

		QList<QStandardItem*> row
		{
			new QStandardItem { e.Entity_.toString () },
			new QStandardItem { e.Additional_ [AN::EF::FullText].toString () },
			new QStandardItem { Util::AN::GetCategoryName (category) },
			new QStandardItem { Util::AN::GetTypeName (type) }
		};
		for (const auto item : row)
			item->setEditable (false);

		row [0]->setData (QVariant::fromValue (e));

		auto possibleFields = Util::GetStdANFields (category) + Util::GetStdANFields (type);
		const auto& sender = e.Additional_ [AN::EF::SenderID].toByteArray ();
		if (const auto iane = qobject_cast<IANEmitter*> (GetProxyHolder ()->GetPluginsManager ()->GetPluginByID (sender)))
			possibleFields += iane->GetANFields ();

		for (const auto& fieldData : possibleFields)
			if (e.Additional_.contains (fieldData.ID_))
			{
				QList<QStandardItem*> subrow
				{
					new QStandardItem { fieldData.Name_ },
					new QStandardItem { e.Additional_ [fieldData.ID_].toString () },
					new QStandardItem { fieldData.Description_ }
				};

				for (const auto item : subrow)
					item->setEditable (false);

				subrow [0]->setData (fieldData.ID_);

				row [0]->appendRow (subrow);
			}

		Model_->insertRow (0, row);
	}

	QAbstractItemModel* UnhandledNotificationsKeeper::GetUnhandledModel () const
	{
		return Model_;
	}

	namespace
	{
		auto BuildHierarchy (const QList<QStandardItem*>& allItems)
		{
			QHash<QStandardItem*, QList<QStandardItem*>> result;
			for (const auto& item : allItems)
				if (const auto& parent = item->parent ())
				{
					if (allItems.contains (parent))
						result [parent] << item;
				}
				else
					result.insert (item, {});
			return result;
		}
	}

	QList<Entity> UnhandledNotificationsKeeper::GetRulesEntities (const QList<QModelIndex>& idxs) const
	{
		QList<Entity> result;

		const auto& allItems = Util::Map (idxs,
				[this] (const QModelIndex& idx) { return Model_->itemFromIndex (idx); });
		const auto& hierarchy = BuildHierarchy (allItems);

		for (const auto& pair : Util::Stlize (hierarchy))
		{
			const auto& entityNode = pair.first;
			const auto& fieldNodes = pair.second;
			auto entity = entityNode->data ().value<Entity> ();
			for (int i = 0; i < entityNode->rowCount (); ++i)
			{
				const auto& fieldNode = entityNode->child (i);
				if (!fieldNodes.contains (fieldNode))
					entity.Additional_.remove (fieldNode->data ().toString ());
			}

			result << entity;
		}

		return result;
	}
}
