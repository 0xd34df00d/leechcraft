/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariantList>
#include <QModelIndexList>
#include <util/network/addresses.h>
#include "xsdconfig.h"

class QHostAddress;
class QStandardItemModel;
class QStandardItem;

namespace LC::Util
{
	class BaseSettingsManager;

	/** @brief Manages an XML settings model with a list of network
	 * interfaces selected by the user from the list of available ones.
	 *
	 * AddressesModelManager uses a BaseSettingsManager instance to store
	 * the interfaces.
	 *
	 * The default list is populated by local-only interfaces.
	 */
	class UTIL_XSD_API AddressesModelManager : public QObject
	{
		Q_OBJECT

		QStandardItemModel * const Model_;
		BaseSettingsManager * const BSM_;
	public:
		/** @brief Constructs the model manager.
		 *
		 * @param[in] bsm The settings manager to be used to store the
		 * list of interfaces.
		 * @param[in] defaultPort The default port used when presenting
		 * interface addition dialog to the user.
		 * @param[in] parent The parent object.
		 */
		AddressesModelManager (BaseSettingsManager *bsm, int defaultPort, QObject *parent = nullptr);

		/** @brief Registers the types used for storage in Qt metasystem.
		 *
		 * This function should be called once during plugin startup
		 * before XML settings system is initialized (that is, before the
		 * corresponding BaseSettingsManager passed to the constructor
		 * of this class is instantiated).
		 */
		static void RegisterTypes ();

		/** @brief Returns the managed model.
		 *
		 * The model contains the list of network interfaces selected by
		 * the user.
		 *
		 * @return The network interfaces model.
		 *
		 * @sa GetAddresses()
		 */
		QAbstractItemModel* GetModel () const;

		/** @brief Returns the list of addresses of interfaces selected
		 * by the user.
		 *
		 * @return The list of addresses of the interfaces selected by
		 * the user.
		 *
		 * @sa GetModel()
		 */
		AddrList_t GetAddresses () const;
	private:
		void SaveSettings () const;
		void AppendRow (const QPair<QString, QString>&);
	public Q_SLOTS:
		/** @brief Invoked by XML settings dialog to add new user-selected
		 * items.
		 *
		 * @param[in] property The name of the property for which new
		 * items should be added.
		 * @param[in] list The list of items to add (corresponding to the
		 * columns in the model).
		 *
		 * @sa removeRequested()
		 */
		void addRequested (const QString& property, const QVariantList& list);

		/** @brief Invoked by XML settings dialog to remove some
		 * user-selected items.
		 *
		 * @param[in] property The name of the property for which new
		 * items should be added.
		 * @param[in] list The list of model indexes to remove.
		 *
		 * @sa addRequested()
		 */
		void removeRequested (const QString& property, const QModelIndexList& list);
	Q_SIGNALS:
		/** @brief Notifies about the changes in the selected interfaces
		 * list.
		 */
		void addressesChanged ();
	};
}
