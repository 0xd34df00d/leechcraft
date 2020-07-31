/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
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
class Q_DECL_EXPORT IImportExport
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

Q_DECLARE_INTERFACE (IImportExport, "org.Deviant.LeechCraft.IImportExport/1.0")

#endif

