/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

namespace LC
{
	/** @brief Describes various USB removable devices.
	 *
	 * The corresponding device model role is CommonDevRole::DevType.
	 *
	 * All device types are expected to return meaningful data for roles
	 * in the CommonDevRole enum.
	 *
	 * @sa CommonDevRole::DevType
	 * @sa CommonDevRole
	 */
	enum DeviceType
	{
		/** @brief A general USB device.
		 *
		 * The device model rows for this USB device are expected to also
		 * return data for roles in the USBDeviceRole enum.
		 *
		 * @sa USBDeviceRole
		 */
		USBDevice,

		/** @brief A mass storage USB device, like a flash drive.
		 *
		 * The device model rows for this USB device are expected to also
		 * return data for roles in the MassStorageRole enum.
		 *
		 * @sa MassStorageRole
		 */
		MassStorage
	};
}
