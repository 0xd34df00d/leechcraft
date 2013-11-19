/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
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

namespace LeechCraft
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

