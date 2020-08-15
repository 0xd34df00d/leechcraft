/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "avatarsmanager.h"
#include <util/threads/futures.h>
#include <util/sll/util.h>
#include <util/sll/qtutil.h>
#include <util/sll/unreachable.h>
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iextselfinfoaccount.h"
#include "avatarsstorage.h"
#include "resourcesmanager.h"
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Azoth
{
	AvatarsManager::AvatarsManager (QObject *parent)
	: QObject { parent }
	, Storage_ { new AvatarsStorage { this } }
	{
		handleCacheSizeChanged ();
		XmlSettingsManager::Instance ().RegisterObject ("AvatarsCacheSize",
				this, "handleCacheSizeChanged");
	}

	namespace
	{
		int Size2Dim (IHaveAvatars::Size size)
		{
			switch (size)
			{
			case IHaveAvatars::Size::Full:
				return 256;
			case IHaveAvatars::Size::Thumbnail:
				return 64;
			}

			Util::Unreachable ();
		}

		std::optional<IHaveAvatars::Size> ChooseSize (IHaveAvatars *iha, IHaveAvatars::Size size)
		{
			if (iha->SupportsSize (size))
				return size;

			for (auto known : { IHaveAvatars::Size::Full, IHaveAvatars::Size::Thumbnail })
				if (iha->SupportsSize (known))
					return known;

			return {};
		}
	}

	QFuture<QImage> AvatarsManager::GetAvatar (QObject *entryObj, IHaveAvatars::Size size)
	{
		const auto defaultAvatarGetter = [size]
		{
			return ResourcesManager::Instance ().GetDefaultAvatar (Size2Dim (size));
		};

		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto iha = qobject_cast<IHaveAvatars*> (entryObj);
		if (!iha)
			return Util::MakeReadyFuture (defaultAvatarGetter ());

		const auto maybeSupportedSize = ChooseSize (iha, size);
		if (!maybeSupportedSize)
			return Util::MakeReadyFuture (defaultAvatarGetter ());

		const auto supportedSize = *maybeSupportedSize;

		const auto& sizes = PendingRequests_.value (entryObj);
		if (sizes.contains (size))
			return sizes.value (size);

		const auto entryId = entry->GetEntryID ();
		auto future = Util::Sequence (entryObj, Storage_->GetAvatar (entry, size))
				.DestructionValue (defaultAvatarGetter) >>
				[=, this] (const MaybeImage& image)
				{
					if (image)
						return Util::MakeReadyFuture (*image);

					auto refreshFuture = iha->RefreshAvatar (supportedSize);
					Util::Sequence (this, refreshFuture) >>
							[=, this] (QImage img)
							{
								if (auto tgtDim = Size2Dim (size); tgtDim < Size2Dim (supportedSize) && !img.isNull ())
									img = img.scaled (tgtDim, tgtDim, Qt::KeepAspectRatio, Qt::SmoothTransformation);
								Storage_->SetAvatar (entryId, size, img);
							};
					return refreshFuture;
				} >>
				[=, this] (QImage image)
				{
					auto& sizes = PendingRequests_ [entryObj];

					sizes.remove (size);
					if (sizes.isEmpty ())
						PendingRequests_.remove (entryObj);

					if (image.isNull ())
						image = defaultAvatarGetter ();

					return Util::MakeReadyFuture (image);
				};
		PendingRequests_ [entryObj] [size] = future;
		return future;
	}

	QFuture<std::optional<QByteArray>> AvatarsManager::GetStoredAvatarData (const QString& entryId, IHaveAvatars::Size size)
	{
		return Storage_->GetAvatar (entryId, size);
	}

	bool AvatarsManager::HasAvatar (QObject *entryObj) const
	{
		const auto iha = qobject_cast<IHaveAvatars*> (entryObj);
		return iha ?
				iha->HasAvatar () :
				false;
	}

	Util::DefaultScopeGuard AvatarsManager::Subscribe (QObject *obj,
			IHaveAvatars::Size size, const AvatarHandler_f& handler)
	{
		const auto id = ++SubscriptionID_;
		Subscriptions_ [obj] [size] [id] = handler;

		return Util::MakeScopeGuard ([=, this]
				{
					auto& objSubscrs = Subscriptions_ [obj];
					auto& sizeSubscrs = objSubscrs [size];

					sizeSubscrs.remove (id);
					if (!sizeSubscrs.isEmpty ())
						return;

					objSubscrs.remove (size);
					if (objSubscrs.isEmpty ())
						Subscriptions_.remove (obj);
				});
	}

	void AvatarsManager::HandleSubscriptions (QObject *entry)
	{
		for (const auto& pair : Util::Stlize (Subscriptions_.value (entry)))
		{
			if (pair.second.isEmpty ())
				continue;

			const auto size = pair.first;
			Util::Sequence (this, GetAvatar (entry, pair.first)) >>
					[this, size, entry] (const std::optional<QImage>& image)
					{
						const auto realImg = image.value_or (QImage {});
						for (const auto& handler : Subscriptions_.value (entry).value (size))
							handler (realImg);
					};
		}
	}

	void AvatarsManager::handleAccount (QObject *accObj)
	{
		connect (accObj,
				SIGNAL (gotCLItems (QList<QObject*>)),
				this,
				SLOT (handleEntries (QList<QObject*>)));

		const auto acc = qobject_cast<IAccount*> (accObj);
		handleEntries (acc->GetCLEntries ());

		if (const auto iesia = qobject_cast<IExtSelfInfoAccount*> (accObj))
			SelfInfo2Account_ [iesia->GetSelfContact ()] = acc;
	}

	void AvatarsManager::handleEntries (const QList<QObject*>& entries)
	{
		for (const auto entryObj : entries)
		{
			const auto iha = qobject_cast<IHaveAvatars*> (entryObj);
			if (!iha)
				continue;

			connect (entryObj,
					SIGNAL (avatarChanged (QObject*)),
					this,
					SLOT (invalidateAvatar (QObject*)));
		}
	}

	void AvatarsManager::invalidateAvatar (QObject *that)
	{
		const auto entry = qobject_cast<ICLEntry*> (that);
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< "object is not an entry:"
					<< sender ()
					<< that;
			return;
		}

		Storage_->DeleteAvatars (entry->GetEntryID ());

		emit avatarInvalidated (that);

		HandleSubscriptions (that);

		if (const auto acc = SelfInfo2Account_.value (that))
			emit accountAvatarInvalidated (acc);
	}

	void AvatarsManager::handleCacheSizeChanged ()
	{
		Storage_->SetCacheSize (XmlSettingsManager::Instance ()
				.property ("AvatarsCacheSize").toInt ());
	}
}
}
