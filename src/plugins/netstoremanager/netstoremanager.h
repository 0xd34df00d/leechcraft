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
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		TabClasses_t GetTabClasses () const;
		void TabOpenRequested (const QByteArray&);

		QSet<QByteArray> GetExpectedPluginClasses () const;
		void AddPlugin (QObject*);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QAbstractItemModel* GetRepresentation () const;

		QStringList GetServiceVariants () const;
		void UploadFile (const QString& filename, const QString& service);
	signals:
		void addNewTab (const QString&, QWidget*);
		void removeTab (QWidget*);
		void changeTabName (QWidget*, const QString&);
		void changeTabIcon (QWidget*, const QIcon&);
		void statusBarChanged (QWidget*, const QString&);
		void raiseTab (QWidget*);

		void fileUploaded (const QString&, const QUrl&);
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

