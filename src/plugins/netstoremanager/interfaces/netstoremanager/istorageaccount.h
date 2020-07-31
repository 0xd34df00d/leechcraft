/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISTORAGEACCOUNT_H
#define PLUGINS_NETSTOREMANAGER_INTERFACES_NETSTOREMANAGER_ISTORAGEACCOUNT_H
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QMetaType>
#include <QtPlugin>
#include <interfaces/structures.h>

namespace LC
{
namespace NetStoreManager
{
	enum AccountFeature
	{
		None = 0x00,
		FileListings = 0x01,
		ProlongateFiles = 0x02
	};

	Q_DECLARE_FLAGS (AccountFeatures, AccountFeature);

	enum class UploadType
	{
		Upload,
		Update
	};

	class IStorageAccount
	{
	public:
		virtual ~IStorageAccount () {}

		virtual QObject* GetParentPlugin () const = 0;
		virtual QObject* GetQObject () = 0;

		virtual QByteArray GetUniqueID () const = 0;

		virtual QString GetAccountName () const = 0;
		virtual AccountFeatures GetAccountFeatures () const = 0;

		virtual void Upload (const QString& filepath,
				const QByteArray& parentId = QByteArray (),
				UploadType ut = UploadType::Upload,
				const QByteArray& id = QByteArray ()) = 0;
		virtual void Download (const QByteArray& id, const QString& filepath,
				TaskParameters tp, bool open) = 0;
	protected:
		virtual void upStatusChanged (const QString& status, const QString& filepath) = 0;
		virtual void upProgress (quint64 done, quint64 total, const QString& filepath) = 0;
		virtual void upError (const QString& error, const QString& filepath) = 0;
		virtual void upFinished (const QByteArray& id, const QString& filepath) = 0;
		virtual void downloadFile (const QUrl& url, const QString& filepath,
				TaskParameters tp, bool open) = 0;
	};
}
}

Q_DECLARE_OPERATORS_FOR_FLAGS (LC::NetStoreManager::AccountFeatures)

Q_DECLARE_INTERFACE (LC::NetStoreManager::IStorageAccount,
		"org.Deviant.LeechCraft.NetStoreManager.IStorageAccount/1.0")
Q_DECLARE_METATYPE (LC::NetStoreManager::IStorageAccount*)

#endif
