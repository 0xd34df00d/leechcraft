/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <optional>
#include <QDir>
#include <QObject>
#include <util/threads/coro/workerthread.h>
#include "interfaces/azoth/core/iavatarsmanager.h"
#include "interfaces/azoth/azothcommon.h"

namespace LC::Azoth
{
	class AvatarsStorage : public QObject
	{
		QDir CacheDir_;
	public:
		explicit AvatarsStorage ();

		void SetAvatar (const GlobalStrongestId& entryId, const QByteArray& imageData);
		std::optional<QByteArray> GetAvatarData (const GlobalStrongestId& entryId) const;

		struct NotStored {};
		struct Empty {};

		template<typename T>
		using Result_t = std::variant<NotStored, Empty, T>;

		static Result_t<QImage> ReadAvatarImage (const GlobalStrongestId&, IAvatarsManager::AvatarSize, QIODevice&);

		Result_t<QImage> GetAvatarImage (const GlobalStrongestId& entryId, IAvatarsManager::AvatarSize size) const;

		void MoveAvatars (const GlobalStrongestId& from, const GlobalStrongestId& to);
		void DeleteAvatars (const GlobalStrongestId& entryId);
	private:
		QString GetPath (const GlobalStrongestId& entryId) const;

		enum class Mode : uint8_t { Read, Write };
		std::unique_ptr<QFile> GetFile (const GlobalStrongestId& entryId, Mode mode) const;
	};

	class AvatarsStorageThread : public Util::Coro::WorkerThread<AvatarsStorage, AvatarsStorageThread, Util::SharedTaskExtension>
	{
	public:
		using WorkerThread::WorkerThread;
	};
}
