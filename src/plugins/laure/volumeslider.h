/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011  Minh Ngo
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

#ifndef PLUGINS_LAURE_VOLUMESLIDER_H
#define PLUGINS_LAURE_VOLUMESLIDER_H

#include <QSlider>
#include <QPixmap>

class QMouseEvent;

namespace LeechCraft
{
namespace Laure
{
	/** @author Minh Ngo <nlminhtl@gmail.com>
	 * @brief An implementation of the Volume slider
	 */
	class VolumeSlider : public QSlider
	{
		Q_OBJECT
		
		QPixmap VolumeSliderInset_, VolumeSliderGradient_;
	public:
		VolumeSlider (QWidget* = 0);
	protected:
		void paintEvent (QPaintEvent *ev);
		void mousePressEvent (QMouseEvent *ev);
		void mouseMoveEvent (QMouseEvent *ev);
	private:
		void GenerateGradient ();
	private slots:
		void slotAnimTimer ();
	};
}
}

#endif // PLUGINS_LAURE_VOLUMESLIDER_H
