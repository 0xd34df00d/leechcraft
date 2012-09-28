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
#include <interfaces/lmp/isyncplugin.h>

namespace LeechCraft
{
namespace LMP
{
namespace DumbSync
{
	class Plugin : public QObject
				 , public IInfo
				 , public IHaveSettings
				 , public IPlugin2
				 , public ILMPPlugin
				 , public ISyncPlugin
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IHaveSettings
				IPlugin2
				LeechCraft::LMP::ILMPPlugin
				LeechCraft::LMP::ISyncPlugin)

		Util::XmlSettingsDialog_ptr XSD_;
		ILMPProxy_ptr LMPProxy_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;

		QSet<QByteArray> GetPluginClasses () const;

		void SetLMPProxy (ILMPProxy_ptr);

		QObject* GetObject ();
		QString GetSyncSystemName () const;
		SyncConfLevel CouldSync (const QString&);
		void Upload (const QString& localPath, const QString& origLocalPath,
				const QString& to, const QString& relPath);
	private slots:
		void handleCopyFinished ();
	signals:
		void uploadFinished (const QString&, QFile::FileError, const QString&);
	};
}
}
}
