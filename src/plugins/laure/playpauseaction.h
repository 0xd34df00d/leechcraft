/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_LAURE_PLAYPAUSEACTION_H
#define PLUGINS_LAURE_PLAYPAUSEACTION_H
#include <QAction>

namespace LeechCraft
{
namespace Laure
{
	class PlayPauseAction : public QAction
	{
		Q_OBJECT

		int Play_;
	public:
		PlayPauseAction (const QString& text, QObject *parent = 0);
		void SetIcon ();
	public slots:
		void handlePause ();
		void handlePlay ();
		void handleTriggered ();
		
	signals:
		void play ();
		void pause();
	};
}
}

#endif // PLUGINS_LAURE_PLAYPAUSECTION_H
