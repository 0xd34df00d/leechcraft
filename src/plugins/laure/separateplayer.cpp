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

#include "separateplayer.h"
#include <QCloseEvent>
#include <QPainter>

namespace LeechCraft
{
namespace Laure
{
	SeparatePlayer::SeparatePlayer (QWidget *parent)
	: QGLWidget (parent)
	, FullScreenMode_ (false)
	{
		setPalette (QPalette (Qt::black));
	}
	
	void SeparatePlayer::closeEvent (QCloseEvent *event)
	{
		emit closed ();
		event->accept ();
	}
	
	void SeparatePlayer::initializeGL ()
	{
		glEnable (GL_MULTISAMPLE);
	}
	
	void SeparatePlayer::paintEvent (QPaintEvent *event)
	{
		QPainter painter (this);
		QString text = tr("Click and drag with the left mouse button "
                      "to rotate the Qt logo.");
		QFontMetrics metrics = QFontMetrics(font());
		int border = qMax(4, metrics.leading());

		QRect rect = metrics.boundingRect(0, 0, width() - 2*border, int(height()*0.125),
                                      Qt::AlignCenter | Qt::TextWordWrap, text);
		painter.setRenderHint(QPainter::TextAntialiasing);
		painter.fillRect(QRect(0, 0, width(), rect.height() + 2*border),
                     QColor(0, 0, 0, 127));
		painter.setPen(Qt::white);
		painter.fillRect(QRect(0, 0, width(), rect.height() + 2*border),
                      QColor(0, 0, 0, 127));
		painter.drawText((width() - rect.width())/2, border,
                      rect.width(), rect.height(),
                      Qt::AlignCenter | Qt::TextWordWrap, text);
		painter.end ();
	}
	
	void SeparatePlayer::resizeGL (int width, int height)
	{
		SetupViewport (width, height);
	}
	
	void SeparatePlayer::SetupViewport (int width, int height)
	{
		int side = qMin (width, height);
		glViewport ((width - side) / 2, (height - side) / 2, side, side);

		glMatrixMode (GL_PROJECTION);
		glLoadIdentity ();
#ifdef QT_OPENGL_ES
		glOrthof (-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#else
		glOrtho (-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#endif
		glMatrixMode (GL_MODELVIEW);
	}
	
	void SeparatePlayer::keyPressEvent (QKeyEvent *event)
	{
		switch (event->key ())
		{
		case Qt::Key_F11:
			if (FullScreenMode_ = !FullScreenMode_)
				showFullScreen ();
			else
				showNormal ();
			break;
		}
	}
}
}