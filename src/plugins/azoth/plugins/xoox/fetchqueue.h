/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_FETCHQUEUE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_FETCHQUEUE_H
#include <functional>
#include <QObject>
#include <QSet>
#include <QStringList>

class QTimer;

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	class FetchQueue : public QObject
	{
		Q_OBJECT

		QTimer *FetchTimer_;
		QStringList Queue_;
		std::function<void (const QString&, bool)> FetchFunction_;
		int PerShot_;
		QSet<QString> Reports_;
	public:
		enum Priority
		{
			PHigh,
			PLow
		};
		FetchQueue (std::function<void (const QString&, bool)> func,
				int timeout, int perShot, QObject* = 0);

		void Schedule (const QString&, Priority = PLow, bool report = false);
		void Clear ();
	private slots:
		void handleFetch ();
	};
}
}
}

#endif
