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

#include "structures.h"
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LCFTP
		{
			bool operator== (const TaskData& t1, const TaskData& t2)
			{
				return t1.ID_ == t2.ID_ &&
					t1.URL_ == t2.URL_ &&
					t1.Filename_ == t2.Filename_ &&
					t1.Internal_ == t2.Internal_ &&
					t1.Direction_ == t2.Direction_ &&
					t1.Paused_ == t2.Paused_;
			}
		};
	};
};

QDataStream& operator<< (QDataStream& out, const LeechCraft::Plugins::LCFTP::TaskData& td)
{
	int version = 1;

	out << version
		<< static_cast<qint8> (td.Direction_)
		<< td.URL_
		<< td.Filename_
		<< td.Internal_
		<< td.Paused_;
	return out;
}

QDataStream& operator>> (QDataStream& in, LeechCraft::Plugins::LCFTP::TaskData& td)
{
	int version;
	in >> version;

	if (version == 1)
	{
		qint8 dir;
		in >> dir
			>> td.URL_
			>> td.Filename_
			>> td.Internal_
			>> td.Paused_;

		td.Direction_ = static_cast<LeechCraft::Plugins::LCFTP::TaskData::Direction> (dir);
	}
	else
		qWarning () << Q_FUNC_INFO
			<< "unknown version"
			<< version;
	return in;
}

