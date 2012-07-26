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

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/ihavesettings.h>
#include <interfaces/iplugin2.h>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/lmp/icloudstorageplugin.h>

namespace LeechCraft
{
namespace LMP
{
namespace MP3Tunes
{
	class AccountsManager;
	class Uploader;

	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public IPlugin2
				 , public ILMPPlugin
				 , public ICloudStoragePlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveSettings
				IPlugin2
				LeechCraft::LMP::ILMPPlugin
				LeechCraft::LMP::ICloudStoragePlugin)

		ICoreProxy_ptr Proxy_;

		AccountsManager *AccMgr_;
		Util::XmlSettingsDialog_ptr XSD_;

		QMap<QString, Uploader*> Uploaders_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;

		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		void SetLMPProxy (ILMPProxy*);

		QObject* GetObject ();
		QString GetCloudName () const;
		QIcon GetCloudIcon () const;
		QStringList GetSupportedFileFormats () const;
		void Upload (const QString& account, const QString& filename);
		QStringList GetAccounts () const;
	signals:
		void uploadFinished (const QString&,
				LeechCraft::LMP::CloudStorageError, const QString&);
		void accountsChanged ();

		void gotEntity (const LeechCraft::Entity&);
		void delegateEntity (const LeechCraft::Entity&, int*, QObject**);
	};
}
}
}
