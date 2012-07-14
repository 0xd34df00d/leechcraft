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

#pragma once

#include <QObject>
#include <QSettings>
#include <interfaces/isyncable.h>
#include "todoitem.h"

namespace LeechCraft
{
namespace Otlozhu
{
	class DeltaGenerator : public QObject
	{
		Q_OBJECT

		QSettings Settings_;
		bool IsEnabled_;
		bool IsMerging_;

		QStringList NewItems_;
		QHash<QString, QVariantMap> Diffs_;
		QStringList RemovedItems_;
	public:
		enum DeltaType
		{
			TodoCreated,
			TodoUpdated,
			TodoRemoved
		};

		DeltaGenerator (QObject* = 0);

		void BeginRecording ();

		Sync::Payloads_t GetAllDeltas ();
		Sync::Payloads_t GetNewDeltas ();
		void PurgeDeltas (quint32 num);
		void Apply (const Sync::Payloads_t&);
	private:
		void ApplyCreated (QDataStream&);
		void ApplyUpdated (QDataStream&);
		void ApplyRemoved (QDataStream&);
	private slots:
		void handleItemAdded (int);
		void handleItemRemoved (int);
		void handleItemDiffGenerated (const QString&, const QVariantMap&);
	};
}
}
