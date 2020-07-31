/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <QMap>
#include <QtCrypto>

namespace LC
{
namespace Azoth
{
	class IAccount;
	class ICLEntry;

	class CryptoManager : public QObject
	{
		Q_OBJECT

		std::unique_ptr<QCA::Initializer> QCAInit_;
		std::unique_ptr<QCA::KeyStoreManager> KeyStoreMgr_;
		std::unique_ptr<QCA::EventHandler> QCAEventHandler_;
		QMap<QString, QString> StoredPublicKeys_;

		CryptoManager ();

		CryptoManager (const CryptoManager&) = delete;
		CryptoManager& operator= (const CryptoManager&) = delete;
	public:
		static CryptoManager& Instance ();

		void Release ();

		void AddEntry (ICLEntry*);
		void AddAccount (IAccount*);

		QList<QCA::PGPKey> GetPublicKeys () const;
		QList<QCA::PGPKey> GetPrivateKeys () const;

		void AssociatePrivateKey (IAccount*, const QCA::PGPKey&) const;
	private:
		void RestoreKeyForAccount (IAccount*);
		void RestoreKeyForEntry (ICLEntry*);
	private slots:
		void handleQCAEvent (int, const QCA::Event&);
		void handleQCABusyFinished ();
	};
}
}
