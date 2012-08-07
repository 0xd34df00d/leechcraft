/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

namespace LeechCraft
{
namespace NetStoreManager
{
	class AccountsManager;
	class UpManager;

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

		void gotEntity (const LeechCraft::Entity&);

		void fileUploaded (const QString&, const QUrl&);
	};
}
}

#endif

