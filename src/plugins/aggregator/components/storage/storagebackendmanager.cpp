/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "storagebackendmanager.h"
#include <QCoreApplication>
#include <QThread>
#include <QtDebug>
#include <util/sll/either.h>
#include <util/sll/qtutil.h>
#include "sqlstoragebackend.h"

namespace LC::Aggregator
{
	StorageBackendManager& StorageBackendManager::Instance ()
	{
		static StorageBackendManager sbm;
		return sbm;
	}

	void StorageBackendManager::Release ()
	{
		if (auto cnt = PrimaryStorageBackend_.use_count (); cnt > 1)
			qWarning () << "primary storage use count is" << cnt;

		PrimaryStorageBackend_.reset ();
	}

	void StorageBackendManager::CreatePrimaryStorage ()
	{
		try
		{
			PrimaryStorageBackend_ = std::make_shared<SQLStorageBackend> ();
		}
		catch (const std::exception& e)
		{
			qWarning () << "unable to create primary storage:" << e.what ();
			throw;
		}

		Register (*PrimaryStorageBackend_);
		emit storageCreated ();
	}

	SQLStorageBackend_ptr StorageBackendManager::MakeStorageBackendForThread () const
	{
		if (QThread::currentThread () == qApp->thread ())
			return PrimaryStorageBackend_;

		auto mgr = std::make_shared<SQLStorageBackend> ("_AuxThread");
		Register (*mgr);
		return mgr;
	}

	void StorageBackendManager::Register (const SQLStorageBackend& backend) const
	{
		connect (&backend,
				&SQLStorageBackend::channelAdded,
				this,
				&StorageBackendManager::channelAdded);
		connect (&backend,
				&SQLStorageBackend::channelUnreadCountUpdated,
				this,
				&StorageBackendManager::channelUnreadCountUpdated);
		connect (&backend,
				&SQLStorageBackend::channelDataUpdated,
				this,
				&StorageBackendManager::channelDataUpdated);
		connect (&backend,
				&SQLStorageBackend::itemReadStatusUpdated,
				this,
				&StorageBackendManager::itemReadStatusUpdated);
		connect (&backend,
				&SQLStorageBackend::itemDataUpdated,
				this,
				&StorageBackendManager::itemDataUpdated);
		connect (&backend,
				&SQLStorageBackend::itemsRemoved,
				this,
				&StorageBackendManager::itemsRemoved);
		connect (&backend,
				&SQLStorageBackend::channelRemoved,
				this,
				&StorageBackendManager::channelRemoved);
		connect (&backend,
				&SQLStorageBackend::feedRemoved,
				this,
				&StorageBackendManager::feedRemoved);
		connect (&backend,
				&SQLStorageBackend::hookItemLoad,
				this,
				&StorageBackendManager::hookItemLoad);
		connect (&backend,
				&SQLStorageBackend::hookItemAdded,
				this,
				&StorageBackendManager::hookItemAdded);
	}
}
