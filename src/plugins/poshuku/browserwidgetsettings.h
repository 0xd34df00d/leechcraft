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

#ifndef PLUGINS_POSHUKU_BROWSERWIDGETSETTINGS_H
#define PLUGINS_POSHUKU_BROWSERWIDGETSETTINGS_H
#include <QTime>
#include <QPoint>
#include <QMetaType>

class QDataStream;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			struct BrowserWidgetSettings
			{
				qreal ZoomFactor_;
				bool NotifyWhenFinished_;
				QTime ReloadInterval_;
				QByteArray WebHistorySerialized_;
				QPoint ScrollPosition_;
			};

			QDataStream& operator<< (QDataStream&, const BrowserWidgetSettings&);
			QDataStream& operator>> (QDataStream&, BrowserWidgetSettings&);
		};
	};
};

Q_DECLARE_METATYPE (LeechCraft::Plugins::Poshuku::BrowserWidgetSettings);

#endif

