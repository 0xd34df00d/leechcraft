/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef INTERFACES_IIMPORTEXPORT_H
#define INTERFACES_IIMPORTEXPORT_H
#include <QByteArray>
#include <QtPlugin>

/** @brief Interface for plugins which can import/export data to
 * persistent storage.
 *
 * If a plugin can save/load its data and settings to the disk or some
 * other persistent storage and wants LeechCraft to know about it, it
 * should implement this interface.
 */
class IImportExport
{
public:
	/** @brief Loads settings.
	 *
	 * Loads the settings from byte array previously retrieved from this
	 * plugin with ExportSettings.
	 *
	 * @param[in] data Byte array with settings.
	 *
	 * @sa ImportData
	 * @sa ExportSettings
	 * @sa ExportData
	 */
	virtual void ImportSettings (const QByteArray& data) = 0;

	/** @brief Loads data.
	 *
	 * Loads the data from byte array previously retrieved from this
	 * plugin with ExportData.
	 *
	 * @param[in] data Byte array with data.
	 *
	 * @sa ImportSettings
	 * @sa ExportSettings
	 * @sa ExportData
	 */
	virtual void ImportData (const QByteArray& data) = 0;

	/** @brief Saves settings.
	 *
	 * Saves the settings into byte array which should be loadable with
	 * ImportSettings().
	 *
	 * @return Byte array with settings.
	 *
	 * @sa ImportSettings
	 * @sa ImportData
	 * @sa ExportData
	 */
	virtual QByteArray ExportSettings () const = 0;

	/** @brief Loads settings.
	 *
	 * Saves the data into byte array which should be loadable with
	 * ImportData().
	 *
	 * @return Byte array with data.
	 *
	 * @sa ImportSettings
	 * @sa ImportData
	 * @sa ExportSettings
	 */
	virtual QByteArray ExportData () const = 0;

	/** @brief Virtual destructor.
	 */
	virtual ~IImportExport () {}
};

Q_DECLARE_INTERFACE (IImportExport, "org.Deviant.LeechCraft.IImportExport/1.0");

#endif

