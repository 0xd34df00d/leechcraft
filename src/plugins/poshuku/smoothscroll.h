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

#ifndef PLUGINS_POSHUKU_SMOOTHSCROLL_H
#define PLUGINS_POSHUKU_SMOOTHSCROLL_H
#include <QPoint>
#include <QTime>

class QWebFrame;
class QMouseEvent;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			class SmoothScrollTicker;

			class SmoothScroll
			{
				friend class SmoothScrollTicker;

				enum State {
					SSteady,
					SPressed,
					SManualScroll,
					SAutoScroll,
					SStop
				} State_;

				int Threshold_;
				QPoint PressPos_;
				QPoint Offset_;
				QPoint Delta_;
				QPoint Speed_;
				SmoothScrollTicker *Ticker_;
				QTime Timestamp_;
				QWebFrame *Target_;
				QList<QEvent*> IgnoreList_;
			public:
				SmoothScroll ();

				int GetThreshold () const;
				void SetThreshold (int);

				void SetTarget (QWebFrame*);

				void HandleMousePress (QMouseEvent*);
				void HandleMouseMove (QMouseEvent*);
				void HandleMouseRelease (QMouseEvent*);
			private:
				QPoint GetScrollOffset () const;
				void SetScrollOffset (const QPoint&);
				void Tick ();
			};
		};
	};
};

#endif

