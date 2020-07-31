/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QList>
#include <QString>
#include <QtPlugin>
#include "devicetypes.h"

class QAbstractItemModel;

/** @brief Interface for classes providing information about removable
 * devices.
 *
 * This interface is to be implemented by plugins that provide
 * information about currently connected USB Mass Storage devices or
 * removable devices in general.
 *
 * The information is provided via the model returned by the
 * GetDevicesModel() method. Each row corresponds to a separate device or
 * its part, like a partition for a flash drive. The model is
 * hierarchical: for example, partitions of a flash drive are children of
 * the row representing the flash drive itself.
 *
 * Each row of the model should contain data for roles defined in the
 * LC::CommonDevRole enum as well as LC::USBDeviceRole or
 * LC::MassStorageRole enums, depending on the device type.
 */
class Q_DECL_EXPORT IRemovableDevManager
{
public:
	virtual ~IRemovableDevManager () {}

	/** @brief Checks whether the plugin can handle the device \em type.
	 *
	 * This function should return whether the device manager plugin
	 * recognizes the given \em type of the devices, like USB mass
	 * storages.
	 *
	 * If a device type is supported, the connected devices of the
	 * corresponding \em type are expected to be found in the model
	 * returned by the GetDevicesModel() method.
	 *
	 * @param[in] type The type of the devices to check.
	 * @return Whether the \em type is recognized and supported by the
	 * plugin.
	 */
	virtual bool SupportsDevType (LC::DeviceType type) const = 0;

	/** @brief Returns the model describing the devices.
	 *
	 * Each row of the model should contain data for roles defined in the
	 * LC::CommonDevRole enum as well as LC::USBDeviceRole or
	 * LC::MassStorageRole enums, depending on the device type.
	 *
	 * The returned model should be flat - in other words, no hierarchy is
	 * assumed to be present.
	 *
	 * @return The model describing the currently attached removable
	 * devices.
	 */
	virtual QAbstractItemModel* GetDevicesModel () const = 0;

	/** @brief Tries to mount the device with the given \em id.
	 *
	 * This function tried to mount the device identified by \em id, if
	 * applicable. The \em id corresponds to the ID contained in the
	 * CommonDevRole::DevID role.
	 *
	 * @param[in] id The identifier (as in CommonDevRole::DevID) of the
	 * device to mount.
	 *
	 * @sa CommonDevRole::DevID
	 */
	virtual void MountDevice (const QString& id) = 0;
};

Q_DECLARE_INTERFACE (IRemovableDevManager, "org.Deviant.LeechCraft.IRemovableDevManager/1.0")
