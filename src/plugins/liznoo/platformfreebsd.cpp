/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
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

#include "platformfreebsd.h"
#include <sys/ioctl.h>
#include <dev/acpica/acpiio.h>
#include <fcntl.h>
#include <QDebug>

namespace LeechCraft
{
namespace Liznoo
{
	PlatformFreeBSD::PlatformFreeBSD (QObject *parent)
	: PlatformLayer (parent)
	{
		timer = new QTimer (this);
		timer->start (updateInterval);
		connect(timer, SIGNAL(timeout()), SLOT(update()));
		acpifd = open("/dev/acpi", O_RDONLY);
		// XXX: no error handling
		QTimer::singleShot (0, this, SIGNAL (started ()));
	}

	void PlatformFreeBSD::Stop ()
	{
		if (timer)
			timer->stop();
	}

	void PlatformFreeBSD::ChangeState (PowerState state)
	{
		// XXX: not implemented
	}

	void PlatformFreeBSD::update ()
	{
		int batteries = 0;
		if (acpifd > 0)
		{
			ioctl(acpifd, ACPIIO_BATT_GET_UNITS, &batteries);
			for (int i = 0; i < batteries; i++)
			{
				acpi_battery_ioctl_arg arg;
				BatteryInfo info;
				int units, capacity, percentage, rate, voltage, remaining_time;
				bool valid = false;
				arg.unit = i;
				if (ioctl (acpifd, ACPIIO_BATT_GET_BIF, &arg) >= 0)
				{
					info.ID_ = QString("Battery %1: %2 %3 %4").arg(i)
									.arg(arg.bif.model)
									.arg(arg.bif.serial)
									.arg(arg.bif.oeminfo);
					info.Technology_ = arg.bif.type;
					units = arg.bif.units;
					capacity = arg.bif.lfcap >= 0 ? arg.bif.lfcap : arg.bif.dcap;
				}
				arg.unit = i;
				if (ioctl (acpifd, ACPIIO_BATT_GET_BATTINFO, &arg) >= 0)
				{
					percentage = arg.battinfo.cap;
					rate = arg.battinfo.rate;
					remaining_time = arg.battinfo.min * 60;
				}
				arg.unit = i;
				if (ioctl (acpifd, ACPIIO_BATT_GET_BST, &arg) >= 0)
				{
					voltage = arg.bst.volt;
					if ((arg.bst.state & ACPI_BATT_STAT_INVALID) != ACPI_BATT_STAT_INVALID &&
						arg.bst.state != ACPI_BATT_STAT_NOT_PRESENT)
					{
						valid = true;
					}
				}

				info.Percentage_ = percentage;
				info.Voltage_ = static_cast<double>(voltage) / 1000;
				switch (units)
				{
				case ACPI_BIF_UNITS_MW:
					info.EnergyRate_ = static_cast<double>(rate) / 1000;
					info.EnergyFull_ = static_cast<double>(capacity) / 1000;
					info.Energy_ = info.EnergyFull_ * percentage / 100;
					break;
				case ACPI_BIF_UNITS_MA:
					info.EnergyRate_ = (static_cast<double>(rate) / 1000) * info.Voltage_;
					info.EnergyFull_ = (static_cast<double>(capacity) / 1000) * info.Voltage_;
					info.Energy_ = info.EnergyFull_ * percentage / 100;
					break;
				default:
					valid = false;
					break;
				}
				if (valid)
				{
					if ((arg.bst.state & ACPI_BATT_STAT_DISCHARG) != 0)
					{
						info.TimeToFull_ = 0;
						info.TimeToEmpty_ = remaining_time;
					}
					else if ((arg.bst.state & ACPI_BATT_STAT_CHARGING) != 0)
					{
						info.TimeToEmpty_ = 0;
						info.TimeToFull_ = (info.EnergyFull_ - info.Energy_) / info.EnergyRate_ * 3600;
					}
					emit batteryInfoUpdated (info);
				}
			}
		}
	}
}
}
