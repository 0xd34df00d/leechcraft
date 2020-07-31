/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "avatarsstorage.h"
#include <QBuffer>
#include <QtConcurrentRun>
#include <QtDebug>
#include <util/threads/futures.h>
#include <util/sll/visitor.h>
#include "avatarsstoragethread.h"
#include "interfaces/azoth/iclentry.h"

namespace LC
{
namespace Azoth
{
	AvatarsStorage::AvatarsStorage (QObject *parent)
	: QObject { parent }
	, StorageThread_ { new AvatarsStorageThread { this } }
	, Cache_ { 5 * 1024 * 1024 }
	{
		StorageThread_->SetAutoQuit (true);
		StorageThread_->start (QThread::IdlePriority);
	}

	namespace
	{
		int GetImageCost (const QImage& image)
		{
			if (image.isNull ())
				return 1;

			return image.width () * image.height () * image.depth () / 8;
		}
	}

	QFuture<void> AvatarsStorage::SetAvatar (const QString& entryId,
			IHaveAvatars::Size size, const QImage& image)
	{
		QByteArray data;
		QBuffer buffer { &data };
		image.save (&buffer, "PNG", 0);

		Cache_.insert ({ entryId, size }, new CacheValue_t { image }, GetImageCost (image));

		return StorageThread_->SetAvatar (entryId, size, data);
	}

	QFuture<void> AvatarsStorage::SetAvatar (const QString& entryId,
			IHaveAvatars::Size size, const QByteArray& data)
	{
		Cache_.insert ({ entryId, size }, new CacheValue_t { data }, data.size ());

		return StorageThread_->SetAvatar (entryId, size, data);
	}

	QFuture<MaybeImage> AvatarsStorage::GetAvatar (const ICLEntry *entry, IHaveAvatars::Size size)
	{
		const auto& entryId = entry->GetEntryID ();
		if (const auto value = Cache_ [{ entryId, size }])
		{
			auto image = Util::Visit (*value,
					[] (const QImage& image) { return image; },
					[] (const QByteArray& array)
					{
						QImage image;
						if (!image.loadFromData (array))
						{
							qWarning () << Q_FUNC_INFO
									<< "unable to load image";
							return QImage {};
						}
						return image;
					});
			CacheValue_t convertedValue { image };
			if (convertedValue.index () != value->index ())
				Cache_.insert ({ entryId, size }, new CacheValue_t { std::move (convertedValue) }, GetImageCost (image));

			return Util::MakeReadyFuture<MaybeImage> (std::move (image));
		}

		const auto& hrId = entry->GetHumanReadableID ();

		return Util::Sequence (this, StorageThread_->GetAvatar (entryId, size)) >>
				[=] (const MaybeByteArray& data)
				{
					if (!data || data->isEmpty ())
						return Util::MakeReadyFuture<MaybeImage> ({});

					QImage image;
					if (!image.loadFromData (*data))
					{
						qWarning () << Q_FUNC_INFO
								<< "unable to load image from data for"
								<< entryId
								<< hrId;
						return Util::MakeReadyFuture<MaybeImage> ({});
					}

					return Util::MakeReadyFuture<MaybeImage> (image);
				};
	}

	QFuture<MaybeByteArray> AvatarsStorage::GetAvatar (const QString& entryId, IHaveAvatars::Size size)
	{
		if (const auto value = Cache_ [{ entryId, size }])
		{
			auto ba = Util::Visit (*value,
					[] (const QByteArray& arr) { return arr; },
					[] (const QImage& image)
					{
						QByteArray data;
						QBuffer buffer { &data };
						image.save (&buffer, "PNG", 0);
						return data;
					});
			return Util::MakeReadyFuture<MaybeByteArray> (std::move (ba));
		}

		return StorageThread_->GetAvatar (entryId, size);
	}

	QFuture<void> AvatarsStorage::DeleteAvatars (const QString& entryId)
	{
		for (const auto& key : Cache_.keys ())
			if (key.first == entryId)
				Cache_.remove (key);

		return StorageThread_->DeleteAvatars (entryId);
	}

	void AvatarsStorage::SetCacheSize (int mibs)
	{
		Cache_.setMaxCost (mibs * 1024 * 1024);
	}
}
}
