/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_NETSTOREMANAGER_NETSTOREMANAGER_H
#define PLUGINS_NETSTOREMANAGER_NETSTOREMANAGER_H
#include <QObject>
#include <QModelIndex>
#include <interfaces/iinfo.h>
#include <interfaces/ihavetabs.h>
#include <interfaces/ipluginready.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/ijobholder.h>
#include <interfaces/iwebfilestorage.h>
#include "interfaces/netstoremanager/isupportfilelistings.h"
#include "syncwidget.h"

namespace LC
{
namespace NetStoreManager
{
	class AccountsManager;
	class UpManager;
	class SyncManager;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveTabs
				 , public IPluginReady
				 , public IHaveSettings
				 , public IJobHolder
				 , public IWebFileStorage
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveTabs
				IPluginReady
				IHaveSettings
				IJobHolder
				IWebFileStorage)

		LC_PLUGIN_METADATA ("org.LeechCraft.NetStoreManager")

		TabClassInfo ManagerTC_;
		Util::XmlSettingsDialog_ptr XSD_;

		AccountsManager *AccountsManager_;
		UpManager *UpManager_;
		SyncManager *SyncManager_;

		ICoreProxy_ptr Proxy_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		TabClasses_t GetTabClasses () const override;
		void TabOpenRequested (const QByteArray&) override;

		QSet<QByteArray> GetExpectedPluginClasses () const override;
		void AddPlugin (QObject*) override;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const override;

		std::vector<IJobHolderRepresentationHandler_ptr> CreateRepresentationHandlers (const ViewCallbacks&) override;

		QStringList GetServiceVariants () const override;
		void UploadFile (const QString& filename, const QString& service) override;
	signals:
		void fileUploaded (const QString&, const QUrl&) override;
	};

// 	QDataStream& operator<< (QDataStream& out, const QList<SyncerInfo>& info);
// 	QDataStream& operator>> (QDataStream& in, QList<SyncerInfo>& info);

	QDataStream& operator<< (QDataStream& out, const SyncerInfo& info);
	QDataStream& operator>> (QDataStream& in, SyncerInfo& info);

	QDataStream& operator<< (QDataStream& out, const Change& change);
	QDataStream& operator>> (QDataStream& in, Change& change);

	QDataStream& operator<< (QDataStream& out, const StorageItem& item);
	QDataStream& operator>> (QDataStream& in, StorageItem& item);
}
}

#endif
