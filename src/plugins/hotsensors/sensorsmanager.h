/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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
#include <sensors/sensors.h>

namespace LeechCraft
{
namespace HotSensors
{
	struct StoredChipName
	{
		QByteArray Prefix_;
		sensors_bus_id Bus_;
		int Addr_;
		QByteArray Path_;

		StoredChipName ();
		StoredChipName (const sensors_chip_name*);

		sensors_chip_name ToSensorsChip ();
	};

	struct StoredSubfeature
	{
		StoredChipName Chip_;
		int SF_;
	};

	struct StoredTemp
	{
		double Max_;
		double Crit_;
		StoredSubfeature SF_;
		QString Name_;
	};

	class SensorsManager : public QObject
	{
		Q_OBJECT

		QList<StoredTemp> Features_;
	public:
		SensorsManager (QObject* = 0);
		~SensorsManager ();
	private:
		void EnumerateSensors ();
	private slots:
		void readTemperatures ();
	};
}
}
