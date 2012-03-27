/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "avatarsstorage.h"
#include <util/util.h>
#include <QTimer>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	AvatarsStorage::AvatarsStorage (QObject *parent)
	: QObject (parent)
	{
		AvatarsDir_ = Util::CreateIfNotExists ("azoth/xoox/hashed_avatars");

		QTimer::singleShot (30000, this, SLOT (collectOldAvatars ()));
	}

	void AvatarsStorage::StoreAvatar (const QImage& image, const QByteArray& hash)
	{
		QFile file (AvatarsDir_.absoluteFilePath (hash));
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< "for writing";
			return;
		}

		image.save (&file, "PNG");
	}

	QImage AvatarsStorage::GetAvatar (const QByteArray& hash) const
	{
		return QImage (AvatarsDir_.absoluteFilePath (hash));
	}

	void AvatarsStorage::collectOldAvatars ()
	{
		auto list = AvatarsDir_.entryList (QDir::Files, QDir::Time | QDir::Reversed);
		while (list.size () > 4000)
			AvatarsDir_.remove (list.takeLast ());
	}
}
}
}
