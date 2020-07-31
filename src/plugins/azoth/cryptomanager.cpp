/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cryptomanager.h"
#include <QSettings>
#include <QCoreApplication>
#include "interfaces/azoth/isupportpgp.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iclentry.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
	CryptoManager::CryptoManager ()
	: QCAInit_ { new QCA::Initializer }
	, KeyStoreMgr_ { new QCA::KeyStoreManager }
	, QCAEventHandler_ { new QCA::EventHandler }
	{
		connect (QCAEventHandler_.get (),
				SIGNAL (eventReady (int, const QCA::Event&)),
				this,
				SLOT (handleQCAEvent (int, const QCA::Event&)));
		if (KeyStoreMgr_->isBusy ())
			connect (KeyStoreMgr_.get (),
					SIGNAL (busyFinished ()),
					this,
					SLOT (handleQCABusyFinished ()),
					Qt::QueuedConnection);
		QCAEventHandler_->start ();
		KeyStoreMgr_->start ();

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth" };
		settings.beginGroup ("PublicEntryKeys");
		for (const auto& entryId : settings.childKeys ())
			StoredPublicKeys_ [entryId] = settings.value (entryId).toString ();
		settings.endGroup ();
	}

	CryptoManager& CryptoManager::Instance ()
	{
		static CryptoManager cm;
		return cm;
	}

	void CryptoManager::Release ()
	{
		QCAEventHandler_.reset ();
		KeyStoreMgr_.reset ();
		QCAInit_.reset ();
	}

	void CryptoManager::AddEntry (ICLEntry *clEntry)
	{
		if (!KeyStoreMgr_->isBusy ())
			RestoreKeyForEntry (clEntry);
	}

	void CryptoManager::AddAccount (IAccount *account)
	{
		if (!KeyStoreMgr_->isBusy ())
			RestoreKeyForAccount (account);
	}

	QList<QCA::PGPKey> CryptoManager::GetPublicKeys () const
	{
		QList<QCA::PGPKey> result;

		QCA::KeyStore store { "qca-gnupg", KeyStoreMgr_.get () };

		for (const auto& entry : store.entryList ())
		{
			const auto& key = entry.pgpPublicKey ();
			if (!key.isNull ())
				result << key;
		}

		return result;
	}

	QList<QCA::PGPKey> CryptoManager::GetPrivateKeys () const
	{
		QList<QCA::PGPKey> result;

		QCA::KeyStore store ("qca-gnupg", KeyStoreMgr_.get ());

		for (const auto& entry : store.entryList ())
		{
			const auto& key = entry.pgpSecretKey ();
			if (!key.isNull ())
				result << key;
		}

		return result;
	}

	void CryptoManager::AssociatePrivateKey (IAccount *acc, const QCA::PGPKey& key) const
	{
		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth" };
		settings.beginGroup ("PrivateKeys");
		if (key.isNull ())
			settings.remove (acc->GetAccountID ());
		else
			settings.setValue (acc->GetAccountID (), key.keyId ());
		settings.endGroup ();
	}

	void CryptoManager::RestoreKeyForAccount (IAccount *acc)
	{
		const auto pgp = qobject_cast<ISupportPGP*> (acc->GetQObject ());
		if (!pgp)
			return;

		QSettings settings { QCoreApplication::organizationName (),
			QCoreApplication::applicationName () + "_Azoth" };
		settings.beginGroup ("PrivateKeys");
		const auto& keyId = settings.value (acc->GetAccountID ()).toString ();
		settings.endGroup ();

		if (keyId.isEmpty ())
			return;

		for (const auto& key : GetPrivateKeys ())
			if (key.keyId () == keyId)
			{
				pgp->SetPrivateKey (key);
				break;
			}
	}

	void CryptoManager::RestoreKeyForEntry (ICLEntry *clEntry)
	{
		if (!StoredPublicKeys_.contains (clEntry->GetEntryID ()))
			return;

		const auto pgp = qobject_cast<ISupportPGP*> (clEntry->GetParentAccount ()->GetQObject ());
		if (!pgp)
		{
			qWarning () << Q_FUNC_INFO
					<< clEntry->GetQObject ()
					<< clEntry->GetParentAccount ()
					<< "doesn't implement ISupportGPG though "
						"we have the key";
			return;
		}

		const auto& keyId = StoredPublicKeys_.take (clEntry->GetEntryID ());
		for (const auto& key : GetPublicKeys ())
			if (key.keyId () == keyId)
			{
				pgp->SetEntryKey (clEntry->GetQObject (), key);
				break;
			}
	}

	void CryptoManager::handleQCAEvent (int id, const QCA::Event& event)
	{
		qDebug () << Q_FUNC_INFO << id << event.type ();
	}

	void CryptoManager::handleQCABusyFinished ()
	{
		for (const auto acc : Core::Instance ().GetAccounts ())
		{
			RestoreKeyForAccount (acc);

			for (const auto entryObj : acc->GetCLEntries ())
			{
				const auto entry = qobject_cast<ICLEntry*> (entryObj);
				if (!entry)
				{
					qWarning () << Q_FUNC_INFO
							<< entry
							<< "doesn't implement ICLEntry";
					continue;
				}

				RestoreKeyForEntry (entry);
			}
		}
	}
}
}
