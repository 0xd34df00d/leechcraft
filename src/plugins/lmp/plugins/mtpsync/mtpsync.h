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
#include <QHash>
#include <libmtp.h>
#include <interfaces/iinfo.h>
#include <interfaces/iplugin2.h>
#include <interfaces/lmp/ilmpplugin.h>
#include <interfaces/lmp/iunmountablesync.h>

namespace LeechCraft
{
namespace LMP
{
namespace MTPSync
{
	class Plugin : public QObject
				 , public IInfo
				 , public IPlugin2
				 , public ILMPPlugin
				 , public IUnmountableSync
	{
		Q_OBJECT
		Q_INTERFACES (IInfo
				IPlugin2
				LeechCraft::LMP::ILMPPlugin
				LeechCraft::LMP::IUnmountableSync)

		ILMPProxy_ptr LMPProxy_;
		UnmountableDevInfos_t Infos_;

		QHash<QString, UnmountableFileInfo> OrigInfos_;
	public:
		void Init (ICoreProxy_ptr proxy);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QSet<QByteArray> GetPluginClasses () const;

		void SetLMPProxy (ILMPProxy_ptr);

		QString GetSyncSystemName () const;
		QObject* GetObject ();
		UnmountableDevInfos_t AvailableDevices () const;
		void SetFileInfo (const QString& origLocalPath, const UnmountableFileInfo& info);
		void Upload (const QString& localPath, const QString& origLocalPath, const QByteArray& to, const QByteArray& storageId);

		void HandleTransfer (const QString&, quint64, quint64);
	private:
		void UploadTo (LIBMTP_mtpdevice_t*, const QByteArray&, const QString&, const QString&);
		LIBMTP_album_t* GetAlbum (LIBMTP_mtpdevice_t*, const UnmountableFileInfo&, uint32_t);
	private slots:
		void pollDevices ();
	signals:
		void availableDevicesChanged ();
		void uploadFinished (const QString&, QFile::FileError, const QString&);
	};
}
}
}
