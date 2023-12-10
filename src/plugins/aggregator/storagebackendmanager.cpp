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
#include "dumbstorage.h"
#include "xmlsettingsmanager.h"

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
			qWarning () << Q_FUNC_INFO
					<< "primary storage use count is"
					<< cnt;

		PrimaryStorageBackend_.reset ();
	}

	StorageBackendManager::StorageCreationResult_t StorageBackendManager::CreatePrimaryStorage ()
	{
		const auto& strType = XmlSettingsManager::Instance ().property ("StorageType").toByteArray ();
		try
		{
			PrimaryStorageBackend_ = StorageBackend::Create (strType);
		}
		catch (const std::runtime_error& s)
		{
			PrimaryStorageBackend_ = std::make_shared<DumbStorage> ();
			return StorageCreationResult_t::Left ({ s.what () });
		}

		const int feedsTable = 2;
		const int channelsTable = 2;
		const int itemsTable = 6;

		auto runUpdate = [this, &strType] (auto updater, const char *suffix, int targetVersion)
		{
			const auto& fullPropName = strType + suffix;
			const auto curVersion = XmlSettingsManager::Instance ().Property (Util::AsStringView (fullPropName), targetVersion).toInt ();
			if (curVersion == targetVersion)
				return true;

			if (!std::invoke (updater, PrimaryStorageBackend_.get (), curVersion))
				return false;

			XmlSettingsManager::Instance ().setProperty (fullPropName, targetVersion);
			return true;
		};

		if (!runUpdate (&StorageBackend::UpdateFeedsStorage, "FeedsTableVersion", feedsTable) ||
			!runUpdate (&StorageBackend::UpdateChannelsStorage, "ChannelsTableVersion", channelsTable) ||
			!runUpdate (&StorageBackend::UpdateItemsStorage, "ItemsTableVersion", itemsTable))
			return StorageCreationResult_t::Left ({ "Unable to update tables" });

		PrimaryStorageBackend_->Prepare ();

		emit storageCreated ();

		return StorageCreationResult_t::Right (PrimaryStorageBackend_);
	}

	bool StorageBackendManager::IsPrimaryStorageCreated () const
	{
		return static_cast<bool> (PrimaryStorageBackend_);
	}

	StorageBackend_ptr StorageBackendManager::MakeStorageBackendForThread () const
	{
		if (QThread::currentThread () == qApp->thread ())
			return PrimaryStorageBackend_;

		const auto& strType = XmlSettingsManager::Instance ().property ("StorageType").toString ();
		try
		{
			auto mgr = StorageBackend::Create (strType, "_AuxThread");
			mgr->Prepare ();
			return mgr;
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot create storage for auxiliary thread";
			return {};
		}
	}

	void StorageBackendManager::Register (const StorageBackend_ptr& backend)
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
