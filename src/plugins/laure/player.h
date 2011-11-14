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

#ifndef PLUGINS_LAURE_PLAYER_H
#define PLUGINS_LAURE_PLAYER_H

#include <QFrame>

class QTime;
class QPushButton;
class QTimer;
class QSlider;

namespace LeechCraft
{
namespace Laure
{
	class VLCWrapper;
	
	/** @brief Provides a video frame.
	 * 
	 * @author Minh Ngo <nlminhtl@gmail.com>
	 */
	class Player : public QFrame
	{
		Q_OBJECT
		
		QTimer *Poller_;
		VLCWrapper *VLCWrapper_;
	public:
		/** @brief Constructs a new PlaybackModeMenu class
		 * with the given parent.
		 */
		Player (QWidget* = 0);
		
		/** @brief Sets a libvlc wrapper.
		 * 
		 * @param[in] core VLCWrapper
		 * 
		 * @sa VLCWrapper
		 */
		void SetVLCWrapper (VLCWrapper *core);
		
		/** @brief Returns current media time in the QTime format.
		 */
		QTime Time ();
		
		/** @brief Returns current media length in the QTime format.
		 */
		QTime Length ();
		
		/** @brief Returns current media position.
		 */
		int Position () const;
	public slots:
		
		/** @brief Sets media position.
		 * 
		 * @param[in] pos Media postion
		 */
		void setPosition (int pos);
	signals:
		/** @brief This signal's emmited to update the GUI interface.
		 */
		void timeout ();
	};
}
}
#endif // PLUGINS_LAURE_PLAYER_H
