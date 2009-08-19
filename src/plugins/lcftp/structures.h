/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#ifndef PLUGINS_LCFTP_STRUCTURES_H
#define PLUGINS_LCFTP_STRUCTURES_H
#include <QUrl>
#include <QString>
#include <QDateTime>
#include <QMetaType>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			struct TaskData
			{
				enum Direction
				{
					DDownload,
					DUpload
				};
				Direction Direction_;

				int ID_;
				QUrl URL_;
				/** If the filename is empty, than only the listing is
				 * fetched.
				 */
				QString Filename_;
				/** If a task is internal, it wouldn't be announced to
				 * the outer world, and the fetched entries wouldn't be
				 * downloaded recursively by the Core.
				 */
				bool Internal_;
				bool Paused_;
			};

			bool operator== (const TaskData&, const TaskData&);

			struct FetchedEntry
			{
				QUrl URL_;
				quint64 Size_;
				QDateTime DateTime_;
				bool IsDir_;
				QString Name_;
				TaskData PreviousTask_;
			};
		}
	};
};

QDataStream& operator<< (QDataStream&, const LeechCraft::Plugins::LCFTP::TaskData&);
QDataStream& operator>> (QDataStream&, LeechCraft::Plugins::LCFTP::TaskData&);

Q_DECLARE_METATYPE (LeechCraft::Plugins::LCFTP::TaskData);
Q_DECLARE_METATYPE (LeechCraft::Plugins::LCFTP::FetchedEntry);

#endif

