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
		QObject* GetQObject ();
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
