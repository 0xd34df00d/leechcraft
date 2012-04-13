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

#include "deltagenerator.h"
#include <QApplication>
#include <QtDebug>
#include "core.h"
#include "todomanager.h"
#include "todostorage.h"

typedef QHash<QString, QVariantMap> VariantMapHash_t;
Q_DECLARE_METATYPE (VariantMapHash_t);

namespace LeechCraft
{
namespace Otlozhu
{
	namespace
	{
		/** @brief Merges to diffs into one.
		 *
		 * Values from the source diff take precedence over into.
		 */
		void MergeDiffs (QVariantMap& into, const QVariantMap& source)
		{
			Q_FOREACH (const QString& key, source.keys ())
				into [key] = source [key];
		}
	}

	DeltaGenerator::DeltaGenerator (QObject *parent)
	: QObject (parent)
	, Settings_ (QSettings::IniFormat,
			QSettings::UserScope,
			QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Otlozhu_Deltas")
	, IsEnabled_ (Settings_.value ("Enabled", false).toBool ())
	, IsMerging_ (false)
	{
		NewItems_ = Settings_.value ("NewItems").value<decltype (NewItems_)> ();
		RemovedItems_ = Settings_.value ("RemovedItems").value<decltype (RemovedItems_)> ();
		Diffs_ = Settings_.value ("Diffs").value<decltype (Diffs_)> ();
	}

	void DeltaGenerator::BeginRecording ()
	{
		IsEnabled_ = true;
		Settings_.setValue ("Enabled", true);

		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		connect (storage,
				SIGNAL (itemAdded (int)),
				this,
				SLOT (handleItemAdded (int)),
				Qt::UniqueConnection);
		connect (storage,
				SIGNAL (itemRemoved (int)),
				this,
				SLOT (handleItemRemoved (int)),
				Qt::UniqueConnection);
		connect (storage,
				SIGNAL (itemDiffGenerated (QString, QVariantMap)),
				this,
				SLOT (handleItemDiffGenerated (QString, QVariantMap)),
				Qt::UniqueConnection);
	}

	Sync::Payloads_t DeltaGenerator::GetAllDeltas ()
	{
		BeginRecording ();

		NewItems_.clear ();
		RemovedItems_.clear ();
		Diffs_.clear ();

		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		const auto& all = storage->GetAllItems ();
		std::transform (all.begin (), all.end (), std::back_inserter (NewItems_),
				[] (decltype (all.front ()) item) { return item->GetID (); });
		const auto& result = GetNewDeltas ();
		NewItems_.clear ();
		return result;
	}

	Sync::Payloads_t DeltaGenerator::GetNewDeltas ()
	{
		Sync::Payloads_t result;

		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		Q_FOREACH (const QString& newId, NewItems_)
		{
			const int pos = storage->FindItem (newId);
			if (pos == -1)
				continue;

			auto item = storage->GetItemAt (pos);
			Sync::Payload pay;
			QDataStream stream (pay.Data_);
			stream << static_cast<quint8> (1)
					<< static_cast<quint8> (TodoCreated)
					<< item->Serialize ();
			result << pay;
		}

		Q_FOREACH (const QString& remId, RemovedItems_)
		{
			Sync::Payload pay;
			QDataStream stream (pay.Data_);
			stream << static_cast<quint8> (1)
					<< static_cast<quint8> (TodoRemoved)
					<< remId;
			result << pay;
		}

		Q_FOREACH (const QString& upId, Diffs_.keys ())
		{
			Sync::Payload pay;
			QDataStream stream (pay.Data_);
			stream << static_cast<quint8> (1)
					<< static_cast<quint8> (TodoUpdated)
					<< upId
					<< Diffs_ [upId];
			result << pay;
		}

		return result;
	}

	void DeltaGenerator::PurgeDeltas (quint32 num)
	{
		quint32 toRemove = std::min (static_cast<quint32> (NewItems_.size ()), num);
		NewItems_ = NewItems_.mid (toRemove);
		num -= toRemove;

		toRemove = std::min (static_cast<quint32> (RemovedItems_.size ()), num);
		RemovedItems_ = RemovedItems_.mid (toRemove);
		num -= toRemove;

		const auto& keys = Diffs_.keys ();
		toRemove = std::min (static_cast<quint32> (keys.size ()), num);

		Q_FOREACH (const auto& key, Diffs_.keys ().mid (0, toRemove))
			Diffs_.remove (key);

		Settings_.setValue ("NewItems", NewItems_);
		Settings_.setValue ("RemovedItems", RemovedItems_);
		Settings_.setValue ("Diffs", QVariant::fromValue (Diffs_));
	}

	void DeltaGenerator::Apply (const Sync::Payloads_t& deltas)
	{
		IsMerging_ = true;
		Q_FOREACH (const auto& delta, deltas)
		{
			QDataStream str (delta.Data_);
			quint8 version = 0;
			str >> version;
			if (version != 1)
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown version"
						<< version;
				continue;
			}

			quint8 action = 0;
			str >> action;

			switch (action)
			{
			case DeltaType::TodoCreated:
				ApplyCreated (str);
				break;
			case DeltaType::TodoRemoved:
				ApplyRemoved (str);
				break;
			case DeltaType::TodoUpdated:
				ApplyUpdated (str);
				break;
			default:
				qWarning () << Q_FUNC_INFO
						<< "unknown delta type";
				break;
			}
		}
		IsMerging_ = false;
	}

	void DeltaGenerator::ApplyCreated (QDataStream& str)
	{
		QByteArray data;
		str >> data;
		auto item = TodoItem::Deserialize (data);
		Core::Instance ().GetTodoManager ()->GetTodoStorage ()->AddItem (item);
	}

	void DeltaGenerator::ApplyUpdated (QDataStream& str)
	{
		QString id;
		QVariantMap thatDiff;
		str >> id >> thatDiff;

		auto storage = Core::Instance ().GetTodoManager ()->GetTodoStorage ();
		const int ourItemId = storage->FindItem (id);
		if (ourItemId == -1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown item"
					<< ourItemId;
			return;
		}
		auto ourItem = storage->GetItemAt (ourItemId);

		if (!Diffs_.contains (id) ||
				Diffs_ [id] == thatDiff)
		{
			ourItem->ApplyDiff (thatDiff);
			storage->HandleUpdated (ourItem);
			Diffs_.remove (id);
			return;
		}

		auto thatVersion = ourItem->Clone ();
		thatVersion->ApplyDiff (thatDiff);

		auto ourDiff = Diffs_ [id];
		const auto& ourDate = ourDiff ["DiffGenerationDate"].toDateTime ();
		const auto& thatDate = thatDiff ["DiffGenerationDate"].toDateTime ();
		if (ourDate > thatDate)
		{
			MergeDiffs (thatDiff, ourDiff);
			ourItem->ApplyDiff (thatDiff);
		}
		else
		{
			MergeDiffs (ourDiff, thatDiff);
			ourItem->ApplyDiff (ourDiff);
		}
		auto newDiff = ourItem->DiffWith (thatVersion);
		newDiff ["DiffGenerationDate"] = QDateTime::currentDateTimeUtc ();
		Diffs_ [id] = newDiff;
		storage->HandleUpdated (ourItem);
	}

	void DeltaGenerator::ApplyRemoved (QDataStream& str)
	{
		QString id;
		str >> id;
		Core::Instance ().GetTodoManager ()->GetTodoStorage ()->RemoveItem (id);
	}

	void DeltaGenerator::handleItemAdded (int pos)
	{
		if (IsMerging_)
			return;

		NewItems_ << Core::Instance ().GetTodoManager ()->
				GetTodoStorage ()->GetItemAt (pos)->GetID ();

		Settings_.setValue ("NewItems", NewItems_);
	}

	void DeltaGenerator::handleItemRemoved (int pos)
	{
		if (IsMerging_)
			return;

		const auto& id = Core::Instance ().GetTodoManager ()->
				GetTodoStorage ()->GetItemAt (pos)->GetID ();
		if (NewItems_.removeAll (id))
		{
			Settings_.setValue ("NewItems", NewItems_);
			return;
		}

		if (Diffs_.remove (id))
			Settings_.setValue ("Diffs", QVariant::fromValue (Diffs_));

		RemovedItems_ << id;
		Settings_.setValue ("RemovedItems", RemovedItems_);
	}

	void DeltaGenerator::handleItemDiffGenerated (const QString& id, const QVariantMap& diff)
	{
		if (IsMerging_)
			return;

		if (NewItems_.contains (id))
			return;

		if (!Diffs_.contains (id))
			Diffs_ [id] = diff;
		else
			MergeDiffs (Diffs_ [id], diff);

		Diffs_ [id] ["DiffGenerationDate"] = QDateTime::currentDateTimeUtc ();
		Settings_.setValue ("Diffs", QVariant::fromValue (Diffs_));
	}
}
}
