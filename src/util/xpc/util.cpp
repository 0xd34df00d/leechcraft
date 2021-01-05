/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <QStandardItem>
#include <util/util.h>
#include <interfaces/idatafilter.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/an/entityfields.h>
#include <interfaces/ipersistentstorageplugin.h>
#include <interfaces/ijobholder.h>

Q_DECLARE_METATYPE (QVariantList*);

namespace LC::Util
{
	Entity MakeAN (const QString& header, const QString& text, Priority priority,
			const QString& senderID, const QString& cat, const QString& type,
			const QString& id, const QStringList& visualPath,
			int delta, int count,
			const QString& fullText, const QString& extendedText)
	{
		auto e = MakeNotification (header, text, priority);
		e.Additional_ [AN::EF::SenderID] = senderID;
		e.Additional_ [AN::EF::EventCategory] = cat;
		e.Additional_ [AN::EF::EventID] = id;
		e.Additional_ [AN::EF::VisualPath] = visualPath;
		e.Additional_ [AN::EF::EventType] = type;
		e.Additional_ [AN::EF::FullText] = fullText.isNull () ? text : fullText;
		e.Additional_ [AN::EF::ExtendedText] = extendedText.isNull () ? text : extendedText;
		if (delta)
			e.Additional_ [AN::EF::DeltaCount] = delta;
		else
			e.Additional_ [AN::EF::Count] = count;
		return e;
	}

	Entity MakeANRule (const QString& title, const QString& senderID,
			const QString& cat, const QStringList& types, AN::NotifyFlags flags,
			bool openConfiguration, const QList<QPair<QString, ANFieldValue>>& fields)
	{
		auto e = MakeNotification (title, {}, {});
		e.Additional_ [AN::EF::SenderID] = senderID;
		e.Additional_ [AN::EF::EventID] = "org.LC.AdvNotifications.RuleRegister";
		e.Additional_ [AN::EF::EventCategory] = cat;
		e.Additional_ [AN::EF::EventType] = types;
		e.Additional_ [AN::EF::OpenConfiguration] = openConfiguration;
		e.Mime_ += "-rule-create";

		for (const auto& field : fields)
			e.Additional_ [field.first] = QVariant::fromValue (field.second);

		if (flags & AN::NotifySingleShot)
			e.Additional_ [AN::EF::IsSingleShot] = true;
		if (flags & AN::NotifyTransient)
			e.Additional_ [AN::EF::NotifyTransient] = true;
		if (flags & AN::NotifyPersistent)
			e.Additional_ [AN::EF::NotifyPersistent] = true;
		if (flags & AN::NotifyAudio)
			e.Additional_ [AN::EF::NotifyAudio] = true;

		return e;
	}

	QList<QObject*> GetDataFilters (const QVariant& data, IEntityManager* manager)
	{
		const auto& e = MakeEntity (data, QString (), {}, "x-leechcraft/data-filter-request");
		const auto& handlers = manager->GetPossibleHandlers (e);

		QList<QObject*> result;
		std::copy_if (handlers.begin (), handlers.end (), std::back_inserter (result),
				[] (QObject *obj) { return qobject_cast<IDataFilter*> (obj); });
		return result;
	}

	Entity MakeEntity (const QVariant& entity,
			const QString& location,
			TaskParameters tp,
			const QString& mime)
	{
		Entity result;
		result.Entity_ = entity;
		result.Location_ = location;
		result.Parameters_ = tp;
		result.Mime_ = mime;
		return result;
	}

	Entity MakeNotification (const QString& header,
			const QString& text, Priority priority)
	{
		Entity result = MakeEntity (header,
				QString (),
				AutoAccept | OnlyHandle,
				"x-leechcraft/notification");
		result.Additional_ ["Text"] = text;
		result.Additional_ ["Priority"] = QVariant::fromValue (priority);
		return result;
	}

	Entity MakeANCancel (const Entity& event)
	{
		Entity e = MakeNotification (event.Entity_.toString (), QString (), Priority::Info);
		e.Additional_ [AN::EF::SenderID] = event.Additional_ [AN::EF::SenderID];
		e.Additional_ [AN::EF::EventID] = event.Additional_ [AN::EF::EventID];
		e.Additional_ [AN::EF::EventCategory] = AN::CatEventCancel;
		return e;
	}

	Entity MakeANCancel (const QString& senderId, const QString& eventId)
	{
		Entity e = MakeNotification (QString (), QString (), Priority::Info);
		e.Additional_ [AN::EF::SenderID] = senderId;
		e.Additional_ [AN::EF::EventID] = eventId;
		e.Additional_ [AN::EF::EventCategory] = AN::CatEventCancel;
		return e;
	}

	QVariant GetPersistentData (const QByteArray& key,
			const ICoreProxy_ptr& proxy)
	{
		const auto& plugins = proxy->GetPluginsManager ()->
				GetAllCastableTo<IPersistentStoragePlugin*> ();
		for (const auto plug : plugins)
		{
			const auto& storage = plug->RequestStorage ();
			if (!storage)
				continue;

			const auto& value = storage->Get (key);
			if (!value.isNull ())
				return value;
		}
		return {};
	}

	void SetJobHolderProgress (const QList<QStandardItem*>& row,
			qint64 done, qint64 total, const QString& text)
	{
		const auto item = row.value (JobHolderColumn::JobProgress);
		if (text.contains ("%1") && text.contains ("%2"))
			item->setText (text.arg (done).arg (total));
		else
			item->setText (text);
		SetJobHolderProgress (item, done, total);
	}

	void SetJobHolderProgress (QStandardItem *item, qint64 done, qint64 total)
	{
		auto data = item->data (JobHolderRole::ProcessState).value<ProcessStateInfo> ();
		data.Done_ = done;
		data.Total_ = total;
		item->setData (QVariant::fromValue (data), JobHolderRole::ProcessState);
	}

	void InitJobHolderRow (const QList<QStandardItem*>& row)
	{
		for (const auto item : row)
		{
			item->setEditable (false);
			item->setData (QVariant::fromValue<JobHolderRow> (JobHolderRow::ProcessProgress),
					CustomDataRoles::RoleJobHolderRow);
		}

		const auto item = row.value (JobHolderColumn::JobProgress);

		const ProcessStateInfo state { 0, 0, {}, ProcessStateInfo::State::Running };
		item->setData (QVariant::fromValue (state), JobHolderRole::ProcessState);
	}
}
