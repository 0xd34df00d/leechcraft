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

#include "vaderutil.h"
#include <QAction>
#include "proto/headers.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Vader
{
namespace VaderUtil
{
	State StatusID2State (quint32 id)
	{
		if (id == Proto::UserState::Online)
			return SOnline;
		else if (id == Proto::UserState::Away)
			return SAway;
		else
			return SOffline;
	}

	QList<QAction*> GetBuddyServices (QObject *receiver, const char *slot)
	{
		QList<QAction*> result;
		QAction *world = new QAction (QObject::tr ("MyWorld@Mail.ru"), receiver);
		world->setProperty ("URL", "http://r.mail.ru/cln3587/my.mail.ru/%2/%1/");
		QObject::connect (world,
				SIGNAL (triggered ()),
				receiver,
				slot);
		result << world;
		QAction *photo = new QAction (QObject::tr ("Photo@Mail.ru"), receiver);
		photo->setProperty ("URL", "http://r.mail.ru/cln3565/foto.mail.ru/%2/%1/");
		QObject::connect (photo,
				SIGNAL (triggered ()),
				receiver,
				slot);
		result << photo;
		QAction *video = new QAction (QObject::tr ("Video@Mail.ru"), receiver);
		video->setProperty ("URL", "http://r.mail.ru/cln3567/video.mail.ru/%2/%1/");
		QObject::connect (video,
				SIGNAL (triggered ()),
				receiver,
				slot);
		result << video;
		QAction *blogs = new QAction (QObject::tr ("Blogs@Mail.ru"), receiver);
		blogs->setProperty ("URL", "http://r.mail.ru/cln3566/blogs.mail.ru/%2/%1/");
		QObject::connect (blogs,
				SIGNAL (triggered ()),
				receiver,
				slot);
		result << blogs;
		return result;
	}
	
	QString SubstituteNameDomain (const QString& string, const QString& fullEmail)
	{
		const QStringList& splitted = fullEmail.split ('@', QString::SkipEmptyParts);
		const QString& name = splitted.value (0);
		QString domain = splitted.value (1);
		if (domain.endsWith (".ru"))
			domain.chop (3);
		return string
				.arg (name)
				.arg (domain);
	}
}
}
}
}
