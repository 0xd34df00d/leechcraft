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

#ifndef PLUGINS_AZOTH_INTERFACES_AZOTHUTIL_H
#define PLUGINS_AZOTH_INTERFACES_AZOTHUTIL_H
#include <QList>
#include <QDateTime>
#include <QtDebug>
#include <interfaces/azoth/imessage.h>

namespace LeechCraft
{
namespace Azoth
{
namespace Util
{
	template<typename T>
	void StandardPurgeMessages (QList<T*>& messages, const QDateTime& before)
	{
		if (!before.isValid ())
		{
			messages.clear ();
			return;
		}

		while (!messages.isEmpty ())
		{
			IMessage *msg = qobject_cast<IMessage*> (messages.at (0));
			if (!msg)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast"
						<< messages.at (0)
						<< "to IMessage";
				continue;
			}
			if (msg->GetDateTime () < before)
				delete messages.takeAt (0);
			else
				break;
		}
	}
}
}
}

#endif
