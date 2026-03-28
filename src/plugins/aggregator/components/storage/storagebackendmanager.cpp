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

	StorageBackendManager::StorageCreationResult_t StorageBackendManager::CreatePrimaryStorage ()
	{
		try
		{
			PrimaryStorageBackend_ = std::make_shared<SQLStorageBackend> ();
		}
		catch (const std::exception& e)
		{
			qWarning () << "unable to create primary storage:" << e.what ();
			return { Util::AsLeft, StorageCreationError { e.what () } };
		}

		Register (PrimaryStorageBackend_);
		emit storageCreated ();
		return Util::Void {};
	}

	bool StorageBackendManager::IsPrimaryStorageCreated () const
	{
		return static_cast<bool> (PrimaryStorageBackend_);
	}

	StorageBackend_ptr StorageBackendManager::MakeStorageBackendForThread () const
	{
		if (QThread::currentThread () == qApp->thread ())
			return PrimaryStorageBackend_;

		auto mgr = std::make_shared<SQLStorageBackend> ("_AuxThread");
		Register (mgr);
		return mgr;
	}

	void StorageBackendManager::Register (const StorageBackend_ptr& backend) const
	{
		auto backendPtr = backend.get ();
		connect (backendPtr,
				&StorageBackend::channelAdded,
				this,
				&StorageBackendManager::channelAdded);
		connect (backendPtr,
				&StorageBackend::channelUnreadCountUpdated,
				this,
				&StorageBackendManager::channelUnreadCountUpdated);
		connect (backendPtr,
				&StorageBackend::channelDataUpdated,
				this,
				&StorageBackendManager::channelDataUpdated);
		connect (backendPtr,
				&StorageBackend::itemReadStatusUpdated,
				this,
				&StorageBackendManager::itemReadStatusUpdated);
		connect (backendPtr,
				&StorageBackend::itemDataUpdated,
				this,
				&StorageBackendManager::itemDataUpdated);
		connect (backendPtr,
				&StorageBackend::itemsRemoved,
				this,
				&StorageBackendManager::itemsRemoved);
		connect (backendPtr,
				&StorageBackend::channelRemoved,
				this,
				&StorageBackendManager::channelRemoved);
		connect (backendPtr,
				&StorageBackend::feedRemoved,
				this,
				&StorageBackendManager::feedRemoved);
		connect (backendPtr,
				&StorageBackend::hookItemLoad,
				this,
				&StorageBackendManager::hookItemLoad);
		connect (backendPtr,
				&StorageBackend::hookItemAdded,
				this,
				&StorageBackendManager::hookItemAdded);
	}
}
