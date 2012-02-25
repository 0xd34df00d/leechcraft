/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011-2012  Minh Ngo
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

#pragma once
#include <QAction>

namespace LeechCraft
{
namespace Laure
{
	/** @brief Provides a play and pause actions in the single QAction.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class PlayPauseAction : public QAction
	{
		Q_OBJECT

		int Play_;
	public:
		/** @brief Constructs a new PlayPauseAction class
		 * with the given text and parent.
		 */
		PlayPauseAction (const QString& text, QObject *parent = 0);	
	private:
		void SetIcon ();
	public slots:
		/** @brief Changes to the pause state.
		 */
		void handlePause ();
		
		/** @brief Changes to the play state.
		 */
		void handlePlay ();
	private slots:
		void handleTriggered ();
	signals:
		/** @brief Is emitted when the state is changed to the
		 * play state.
		 */
		void play ();
		
		/** @brief Is emitted when the state is changed to the
		 * pause state.
		 */
		void pause();
	};
}
}
