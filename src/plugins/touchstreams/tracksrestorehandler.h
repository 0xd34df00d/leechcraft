/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QHash>
#include <QFutureInterface>
#include <interfaces/media/audiostructs.h>
#include <interfaces/media/irestorableradiostationprovider.h>

template<typename>
class QFuture;

class QNetworkAccessManager;
class QNetworkReply;

namespace LC
{
namespace Util
{
namespace SvcAuth
{
	class VkAuthManager;
}

class QueueManager;
}

namespace TouchStreams
{
	class TracksRestoreHandler : public QObject
	{
		Q_OBJECT

		Util::SvcAuth::VkAuthManager * const AuthMgr_;
		Util::QueueManager * const Queue_;
		QNetworkAccessManager * const NAM_;

		const QHash<QString, QStringList> IDs_;
		int PendingRequests_ = IDs_.size ();

		QFutureInterface<Media::RadiosRestoreResult_t> FutureIface_;
		Media::RadiosRestoreResult_t Result_;
	public:
		TracksRestoreHandler (const QStringList&, QNetworkAccessManager *nam,
				Util::SvcAuth::VkAuthManager*, Util::QueueManager*, QObject* = nullptr);

		QFuture<Media::RadiosRestoreResult_t> GetFuture ();
	private:
		void Request (const QString&);
		void HandleReplyFinished (QNetworkReply*);
		void NotifyFuture ();
	};
}
}
