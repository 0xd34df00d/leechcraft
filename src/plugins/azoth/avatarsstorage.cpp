/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "avatarsstorage.h"
#include <QCryptographicHash>
#include <QImage>
#include <QImageReader>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>
#include <util/sys/paths.h>
#include "util/azoth/entryidops.h"
#include "util/azoth/util.h"

namespace LC::Azoth
{
	AvatarsStorage::AvatarsStorage ()
	: CacheDir_ { Util::GetUserDir (Util::UserDir::Cache, "azoth/avatars") }
	{
	}

	namespace
	{
		QString GetFilename (const GlobalStrongestId& entryId)
		{
			const auto bytes = SerializeBytes (entryId);
			return QString::fromLatin1 (QCryptographicHash::hash (bytes, QCryptographicHash::Sha1).toHex ()) + ".bin"_qs;
		}
	}

	void AvatarsStorage::SetAvatar (const GlobalStrongestId& entryId, const QByteArray& imageData)
	{
		auto file = GetFile (entryId, Mode::Write);
		if (!file)
			return;
		if (file->write (imageData) != imageData.size () || !file->flush ())
		{
			qWarning () << "unable to write" << file->fileName () << file->errorString ();
			file->remove ();
		}
	}

	std::optional<QByteArray> AvatarsStorage::GetAvatarData (const GlobalStrongestId& entryId) const
	{
		const auto file = GetFile (entryId, Mode::Read);
		if (!file)
			return {};
		return file->readAll ();
	}

	AvatarsStorage::Result_t<QImage> AvatarsStorage::ReadAvatarImage (const GlobalStrongestId& entryId,
			IAvatarsManager::AvatarSize size, QIODevice& device)
	{
		QImageReader reader { &device };

		const auto origSize = reader.size ();
		if (!origSize.isValid ())
			return Empty {};

		const auto area = static_cast<quint64> (origSize.width ()) * origSize.height ();
		constexpr auto depth = 4;
		if (area * depth >= 10 * 1024 * 1024)
		{
			qWarning () << "avatar too large for" << entryId;
			return Empty {};
		}

		Util::Visit (size,
				[] (IAvatarsManager::OriginalSize) {},
				[&] (IAvatarsManager::Scaled scaled)
				{
					reader.setScaledSize (origSize.scaled (scaled.Dim_, scaled.Dim_, Qt::KeepAspectRatio));
				});

		auto img = reader.read ();
		if (img.isNull ())
		{
			qWarning () << "unable to load avatar for" << entryId;
			return Empty {};
		}
		return img;
	}

	AvatarsStorage::Result_t<QImage> AvatarsStorage::GetAvatarImage (const GlobalStrongestId& entryId, IAvatarsManager::AvatarSize size) const
	{
		const auto file = GetFile (entryId, Mode::Read);
		if (!file)
			return NotStored {};
		return ReadAvatarImage (entryId, size, *file);
	}

	void AvatarsStorage::MoveAvatars (const GlobalStrongestId& from, const GlobalStrongestId& to)
	{
		DeleteAvatars (to);

		QFile file { GetPath (from) };
		const auto& targetName = GetPath (to);
		if (file.exists () && !file.rename (targetName))
			qWarning () << "unable to rename" << file.fileName () << "to" << targetName << file.errorString ();
	}

	void AvatarsStorage::DeleteAvatars (const GlobalStrongestId& entryId)
	{
		if (QFile file { GetPath (entryId) };
			file.exists () && !file.remove ())
			qWarning () << "unable to remove" << file.fileName () << file.errorString ();
	}

	QString AvatarsStorage::GetPath (const GlobalStrongestId& entryId) const
	{
		return CacheDir_.filePath (GetFilename (entryId));
	}

	std::unique_ptr<QFile> AvatarsStorage::GetFile (const GlobalStrongestId& entryId, Mode mode) const
	{
		auto file = std::make_unique<QFile> (GetPath (entryId));
		switch (mode)
		{
		case Mode::Read:
			if (!file->exists ())
				return {};
			if (!file->open (QIODevice::ReadOnly))
			{
				qWarning () << "unable to read" << file->fileName () << file->errorString ();
				return {};
			}
			return file;
		case Mode::Write:
			if (!file->open (QIODevice::WriteOnly))
			{
				qWarning () << "unable to write" << file->fileName () << file->errorString ();
				return {};
			}
			return file;
		}

		std::unreachable ();
	}
}
