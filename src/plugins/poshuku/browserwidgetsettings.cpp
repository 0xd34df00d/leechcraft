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

#include "browserwidgetsettings.h"
#include <QDataStream>
#include <QtDebug>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			QDataStream& operator<< (QDataStream& out, const BrowserWidgetSettings& s)
			{
				qint8 version = 3;
				out << version
					<< s.ZoomFactor_
					<< s.NotifyWhenFinished_
					<< s.ReloadInterval_
					<< s.WebHistorySerialized_
					<< s.ScrollPosition_;
				return out;
			}

			QDataStream& operator>> (QDataStream& in, BrowserWidgetSettings& s)
			{
				qint8 version;
				in >> version;
				if (version >= 1)
					in >> s.ZoomFactor_
						>> s.NotifyWhenFinished_
						>> s.ReloadInterval_;
				if (version >= 2)
					in >> s.WebHistorySerialized_;
				if (version >= 3)
					in >> s.ScrollPosition_;

				if (version > 3 || version < 1)
					qWarning () << Q_FUNC_INFO
						<< "unknown version"
						<< version;
				return in;
			}
		};
	};
};

