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

#include "smoothscroll.h"

#include <QObject>
#include <QBasicTimer>
#include <QWebFrame>
#include <QMouseEvent>
#include <QApplication>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class SmoothScrollTicker : public QObject
			{
				SmoothScroll *Scroll_;
				QBasicTimer Timer_;
			public:
				SmoothScrollTicker (SmoothScroll *scr)
				: Scroll_ (scr)
				{
				}

				void Start (int interval)
				{
					if (!Timer_.isActive ())
						Timer_.start (interval, this);
				}

				void Stop ()
				{
					Timer_.stop ();
				}
			private:
				void timerEvent (QTimerEvent*)
				{
					Scroll_->Tick ();
				}
			};

			SmoothScroll::SmoothScroll ()
			: State_ (SSteady)
			, Threshold_ (10)
			, Ticker_ (new SmoothScrollTicker (this))
			, Timestamp_ (QTime::currentTime ())
			, Target_ (0)
			{
			}

			int SmoothScroll::GetThreshold () const
			{
				return Threshold_;
			}

			void SmoothScroll::SetThreshold (int thr)
			{
				if (thr >= 0)
					Threshold_ = thr;
			}

			void SmoothScroll::SetTarget (QWebFrame *t)
			{
				Target_ = t;
			}

			namespace
			{
				QPoint Deaccelerate (const QPoint& speed, int a = 1, int max = 64)
				{
					int x = qBound (-max, speed.x (), max);
					int y = qBound (-max, speed.y (), max);
					x = !x ? x : (x > 0) ? qMax (0, x - a) : qMin (0, x + a);
					y = !y ? y : (y > 0) ? qMax (0, y - a) : qMin (0, y + a);
					return QPoint (x, y);
				}
			};

			void SmoothScroll::HandleMousePress (QMouseEvent *event)
			{
				event->ignore ();

				if (event->button () != Qt::LeftButton)
					return;

				if (IgnoreList_.removeAll (event))
					return;

				switch (State_)
				{
					case SSteady:
						event->accept ();
						State_ = SPressed;
						PressPos_ = event->pos ();
						break;
					case SAutoScroll:
						event->accept ();
						State_ = SStop;
						Speed_ = QPoint (0, 0);
						PressPos_ = event->pos ();
						Offset_ = GetScrollOffset ();
						Ticker_->Stop ();
						break;
					default:
						break;
				}
			}

			void SmoothScroll::HandleMouseRelease (QMouseEvent *event)
			{
				event->ignore ();

				if (event->button () != Qt::LeftButton)
					return;

				if (IgnoreList_.removeAll (event))
					return;

				QPoint delta;
				switch (State_)
				{
					case SPressed:
						event->accept ();
						State_ = SSteady;
						if (Target_)
						{
							QMouseEvent *e1 = new QMouseEvent (QEvent::MouseButtonPress,
									PressPos_, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
							QMouseEvent *e2 = new QMouseEvent (*event);
							IgnoreList_ << e1 << e2;
							QApplication::postEvent (Target_, e1);
							QApplication::postEvent (Target_, e2);
						}
						break;
					case SManualScroll:
						event->accept ();
						delta = event->pos () - PressPos_;
						if (Timestamp_.elapsed () > 100)
						{
							Timestamp_ = QTime::currentTime ();
							Speed_ = delta - Delta_;
							Delta_ = delta;
						}
						Offset_ = GetScrollOffset ();
						PressPos_ = event->pos ();
						if (Speed_ == QPoint (0, 0))
							State_ = SSteady;
						else
						{
							Speed_ /= 4;
							State_ = SAutoScroll;
							Ticker_->Start (10);
						}
						break;
					case SStop:
						event->accept ();
						State_ = SSteady;
						Offset_ = GetScrollOffset ();
						break;
					default:
						break;
				}
			}

			void SmoothScroll::HandleMouseMove (QMouseEvent *event)
			{
				event->ignore ();
				if (!(event->buttons () & Qt::LeftButton))
					return;

				if (IgnoreList_.removeAll (event))
					return;

				QPoint delta;
				switch (State_)
				{
					case SPressed:
					case SStop:
						delta = event->pos () - PressPos_;
						if (abs (delta.x ()) > Threshold_ ||
								abs (delta.y ()) > Threshold_)
						{
							Timestamp_ = QTime::currentTime ();
							State_ = SManualScroll;
							Delta_ = QPoint (0, 0);
							PressPos_ = event->pos ();
							event->accept ();
						}
						break;
					case SManualScroll:
						event->accept ();
						delta = event->pos () - PressPos_;
						SetScrollOffset (Offset_ - delta);
						if (Timestamp_.elapsed () > 100)
						{
							Timestamp_ = QTime::currentTime ();
							Speed_ = delta - Delta_;
							Delta_ = delta;
						}
					default:
						break;
				}
			}

			QPoint SmoothScroll::GetScrollOffset () const
			{
				return Target_->scrollPosition ();
			}

			void SmoothScroll::SetScrollOffset (const QPoint& p)
			{
				Target_->setScrollPosition (p);
			}

			void SmoothScroll::Tick ()
			{
				if (State_ == SAutoScroll)
				{
					Speed_ = Deaccelerate (Speed_);
					SetScrollOffset (Offset_ - Speed_);
					Offset_ = GetScrollOffset ();
					if (Speed_ == QPoint (0, 0))
					{
						State_ == SSteady;
						Ticker_->Stop ();
					}
				}
				else
					Ticker_->Stop ();
			}
		};
	};
};

