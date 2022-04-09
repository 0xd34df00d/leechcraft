/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "eventssettingsmanager.h"
#include <QStandardItemModel>
#include <QCoreApplication>
#include <QSettings>
#include <QtDebug>
#include <xmlsettingsdialog/datasourceroles.h>
#include <util/xpc/stdanfields.h>
#include <util/xpc/anutil.h>
#include <util/sll/prelude.h>
#include <interfaces/an/constants.h>

namespace LC
{
namespace Azoth
{
namespace Tracolor
{
	namespace
	{
		enum Role
		{
			EventId = Qt::UserRole + 1
		};
	}

	EventsSettingsManager::EventsSettingsManager (QObject *parent)
	: QObject { parent }
	, Model_ { new QStandardItemModel { this } }
	{
		Model_->setHorizontalHeaderLabels ({ tr ("Event"), tr ("Color") });
		Model_->setHeaderData (0, Qt::Horizontal,
				DataSources::DataFieldType::Enum,
				DataSources::DataSourceRole::FieldType);
		Model_->setHeaderData (0, Qt::Horizontal,
				true,
				DataSources::DataSourceRole::FieldNonModifiable);
		Model_->setHeaderData (1, Qt::Horizontal,
				DataSources::DataFieldType::Color,
				DataSources::DataSourceRole::FieldType);
		connect (Model_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged ()));

		LoadSettings ();
	}

	QMap<QString, EventsSettingsManager::EventInfo> EventsSettingsManager::GetEnabledEvents () const
	{
		return EnabledEvents_;
	}

	QAbstractItemModel* EventsSettingsManager::GetModel () const
	{
		return Model_;
	}

	void EventsSettingsManager::AppendRow (const QString& eventId,
			const QColor& color, bool isEnabled)
	{
		if (eventId.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty event ID";
			return;
		}

		const QList<QStandardItem*> row
		{
			new QStandardItem { Util::AN::GetTypeName (eventId) },
			new QStandardItem { color.name () }
		};
		for (auto item : row)
			item->setEditable (false);
		row.first ()->setData (eventId, Role::EventId);
		row.first ()->setCheckable (true);
		row.first ()->setCheckState (isEnabled ? Qt::Checked : Qt::Unchecked);
		row.value (1)->setForeground (color);
		Model_->appendRow (row);
	}

	void EventsSettingsManager::LoadSettings ()
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Tracolor" };
		if (!settings.childGroups ().contains ("Events"))
		{
			LoadDefaultSettings ();
			return;
		}

		const auto eventsCount = settings.beginReadArray ("Events");
		for (int i = 0; i < eventsCount; ++i)
		{
			settings.setArrayIndex (i);

			const auto isEnabled = settings.value ("IsEnabled").toBool ();
			const auto& event = settings.value ("Event").toString ();
			const auto& color = settings.value ("Color").toString ();

			AppendRow (event, QColor { color }, isEnabled);
		}
		settings.endArray ();

		RebuildEnabledEvents ();
		RebuildAddableEvents ();
	}

	void EventsSettingsManager::LoadDefaultSettings ()
	{
		AppendRow (AN::TypeIMMUCMsg, "green");
		AppendRow (AN::TypeIMIncMsg, "magenta");
		AppendRow (AN::TypeIMMUCHighlight, "red");
		AppendRow (AN::TypeIMStatusChange, "cyan", false);

		RebuildEnabledEvents ();
		RebuildAddableEvents ();
	}

	void EventsSettingsManager::RebuildEnabledEvents ()
	{
		EnabledEvents_.clear ();

		for (int i = 0, rc = Model_->rowCount (); i < rc; ++i)
		{
			const auto firstItem = Model_->item (i, 0);
			if (firstItem->checkState () != Qt::Checked)
				continue;

			const auto& id = firstItem->data (Role::EventId).toString ();
			const auto& color = Model_->item (i, 1)->text ();
			EnabledEvents_ [id] = EventInfo { color };
		}

		emit eventsSettingsChanged ();
	}

	void EventsSettingsManager::RebuildAddableEvents ()
	{
		QList<QString> remainingEvents
		{
			AN::TypeIMMUCMsg,
			AN::TypeIMIncMsg,
			AN::TypeIMMUCHighlight,
			AN::TypeIMStatusChange,
			AN::TypeIMEventTuneChange,
			AN::TypeIMEventMoodChange,
			AN::TypeIMEventActivityChange,
			AN::TypeIMEventLocationChange
		};

		for (int i = 0, rc = Model_->rowCount (); i < rc; ++i)
			remainingEvents.removeOne (Model_->item (i)->data (Role::EventId).toString ());

		const auto& map = Util::Map (remainingEvents,
				[] (const QString& eventId)
				{
					return QVariant::fromValue<DataSources::EnumValueInfo> ({
								.Name_ = Util::AN::GetTypeName (eventId),
								.UserData_ = eventId,
							});
				});
		Model_->setHeaderData (0, Qt::Horizontal,
				map,
				DataSources::DataSourceRole::FieldValues);
	}

	void EventsSettingsManager::addRequested (const QString&, const QVariantList& datas)
	{
		AppendRow (datas.value (0).toString (), datas.value (1).value<QColor> ());

		saveSettings ();
		RebuildEnabledEvents ();
		RebuildAddableEvents ();
	}

	void EventsSettingsManager::modifyRequested (const QString&, int rowIdx, const QVariantList& datas)
	{
		const auto colorItem = Model_->item (rowIdx, 1);
		const auto& color = datas.value (1).value<QColor> ();

		disconnect (Model_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged ()));
		colorItem->setText (color.name ());
		colorItem->setForeground (color);
		connect (Model_,
				SIGNAL (itemChanged (QStandardItem*)),
				this,
				SLOT (handleItemChanged ()));

		saveSettings ();
		RebuildEnabledEvents ();
	}

	void EventsSettingsManager::removeRequested (const QString&, const QModelIndexList& indexes)
	{
		for (const auto& index : indexes)
			Model_->removeRow (index.row ());

		saveSettings ();
		RebuildEnabledEvents ();
		RebuildAddableEvents ();
	}

	void EventsSettingsManager::saveSettings ()
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Tracolor" };
		settings.beginWriteArray ("Events");
		for (int i = 0, rc = Model_->rowCount (); i < rc; ++i)
		{
			settings.setArrayIndex (i);

			settings.setValue ("IsEnabled", Model_->item (i, 0)->checkState () == Qt::Checked);
			settings.setValue ("Event", Model_->item (i, 0)->data (Role::EventId));
			settings.setValue ("Color", Model_->item (i, 1)->text ());
		}
		settings.endArray ();
	}

	void EventsSettingsManager::handleItemChanged ()
	{
		saveSettings ();
		RebuildEnabledEvents ();
	}
}
}
}
