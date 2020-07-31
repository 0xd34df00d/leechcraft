/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QFutureInterface>
#include <util/sll/either.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/media/iaudiopile.h>

namespace LC
{
namespace Util
{
	class QueueManager;

	namespace SvcAuth
	{
		class VkAuthManager;
	}
}

namespace TouchStreams
{
	class AuthManager;

	class AudioSearch : public QObject
	{
		ICoreProxy_ptr Proxy_;
		Util::QueueManager *Queue_;

		const Media::AudioSearchRequest Query_;

		QFutureInterface<Media::IAudioPile::Result_t> Promise_;
	public:
		AudioSearch (ICoreProxy_ptr, const Media::AudioSearchRequest&,
				Util::SvcAuth::VkAuthManager*, Util::QueueManager*, QObject* = nullptr);

		QFuture<Media::IAudioPile::Result_t> GetFuture ();
	private:
		void HandleGotReply (const QByteArray&);
		void HandleGotAuthKey (const QString&);
	};
}
}
