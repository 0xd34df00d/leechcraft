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

#include "keyinterceptor.h"
#include <QKeyEvent>
#include "playerwidget.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LMP
		{
			KeyInterceptor::KeyInterceptor (PlayerWidget *p, QObject *parent)
			: QObject (parent)
			, Player_ (p)
			{
			}

			KeyInterceptor::~KeyInterceptor ()
			{
			}

			bool KeyInterceptor::eventFilter (QObject *obj, QEvent *e)
			{
				if (e->type () == QEvent::KeyPress)
				{
					keyPressEvent (dynamic_cast<QKeyEvent*> (e));
					return true;
				}
				else if (e->type () == QEvent::KeyRelease)
				{
					keyReleaseEvent (dynamic_cast<QKeyEvent*> (e));
					return true;
				}
				return QObject::eventFilter (obj, e);
			}

			void KeyInterceptor::keyPressEvent (QKeyEvent *e)
			{
				if (e->text () == "*")
					Player_->incrementVolume ();
				else if (e->text () == "/")
					Player_->decrementVolume ();
				else if (e->key () == Qt::Key_Right)
					Player_->Forward (PlayerWidget::SkipLittle);
				else if (e->key () == Qt::Key_Left)
					Player_->Rewind (PlayerWidget::SkipLittle);
				else if (e->key () == Qt::Key_Up)
					Player_->Forward (PlayerWidget::SkipMedium);
				else if (e->key () == Qt::Key_Down)
					Player_->Rewind (PlayerWidget::SkipMedium);
				else if (e->key () == Qt::Key_PageUp)
					Player_->Forward (PlayerWidget::SkipALot);
				else if (e->key () == Qt::Key_PageDown)
					Player_->Rewind (PlayerWidget::SkipALot);
			}

			void KeyInterceptor::keyReleaseEvent (QKeyEvent *e)
			{
				if (e->key () == Qt::Key_Return ||
						e->key () == Qt::Key_Enter)
					Player_->toggleFullScreen ();
				else if (e->key () == Qt::Key_Space)
					Player_->togglePause ();
			}
		};
	};
};


