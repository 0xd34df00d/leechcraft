/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 * Copyright (C) 2012       Maxim Ignatenko
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "freebsdplatform.h"
#include <sys/ioctl.h>
#include <dev/acpica/acpiio.h>
#include <fcntl.h>
#include <errno.h>
#include <QTimer>

namespace LC
{
namespace Liznoo
{
namespace Battery
{
	FreeBSDPlatform::FreeBSDPlatform (QObject *parent)
	: BatteryPlatform { parent }
	, Timer_ { new QTimer { this } }
	, ACPIfd_ { "/dev/acpi", O_RDONLY }
	{
		Timer_->start (10 * 1000);
		connect (Timer_,
				SIGNAL (timeout ()),
				this,
				SLOT (update ()));
	}

	void FreeBSDPlatform::update ()
	{
		int batteries = 0;
		if (!ACPIfd_)
			return;

		ioctl (ACPIfd_, ACPIIO_BATT_GET_UNITS, &batteries);
		for (int i = 0; i < batteries; i++)
		{
			acpi_battery_ioctl_arg arg;
			BatteryInfo info;
			int units = 0, capacity = 0, designCapacity = 0, percentage = 0, rate = 0, voltage = 0, remaining_time = 0;
			bool valid = false;
			arg.unit = i;
			if (ioctl (ACPIfd_, ACPIIO_BATT_GET_BIF, &arg) >= 0)
			{
				info.ID_ = QString("%1 %2 %3")
								.arg (arg.bif.model)
								.arg (arg.bif.serial)
								.arg (arg.bif.oeminfo);
				info.Technology_ = arg.bif.type;
				units = arg.bif.units;
				capacity = arg.bif.lfcap;
				designCapacity = arg.bif.dcap;
			}
			arg.unit = i;
			if (ioctl (ACPIfd_, ACPIIO_BATT_GET_BATTINFO, &arg) >= 0)
			{
				percentage = arg.battinfo.cap;
				rate = arg.battinfo.rate;
				remaining_time = arg.battinfo.min * 60;
			}
			arg.unit = i;
			if (ioctl (ACPIfd_, ACPIIO_BATT_GET_BST, &arg) >= 0)
			{
				voltage = arg.bst.volt;
				if ((arg.bst.state & ACPI_BATT_STAT_INVALID) != ACPI_BATT_STAT_INVALID &&
					arg.bst.state != ACPI_BATT_STAT_NOT_PRESENT)
					valid = true;
			}

			info.Percentage_ = percentage;
			info.Voltage_ = voltage / 1000.0;
			info.EnergyRate_ = rate / 1000.0;
			info.EnergyFull_ = capacity / 1000.0;
			info.DesignEnergyFull_ = designCapacity / 1000.0;
			if (units == ACPI_BIF_UNITS_MA)
			{
				info.EnergyRate_ *= info.Voltage_;
				info.EnergyFull_ *= info.Voltage_;
				info.DesignEnergyFull_ *= info.Voltage_;
			}

			info.Energy_ = info.EnergyFull_ * percentage / 100;

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
