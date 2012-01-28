/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#ifndef PLUGINS_AZOTH_ANIMATEDICONMANAGER_H
#define PLUGINS_AZOTH_ANIMATEDICONMANAGER_H
#include <boost/function.hpp>
#include <QObject>
#include <QImage>
#include <QHash>
#include <QFile>
#include <QTimerEvent>
#include <QIcon>
#include <QImageReader>

namespace LeechCraft
{
namespace Azoth
{
	template<typename T>
	class AnimatedIconManager : public QObject
	{
		struct IconInfo
		{
			int CurrentFrame_;
			QList<QImage> Images_;
			int TimerID_;
		};
		QHash<T, IconInfo> Object2Icon_;
		QHash<int, T> Timer2Object_;
	public:
		typedef boost::function<void (T, const QIcon&)> IconSetter_t;
	private:
		IconSetter_t Setter_;
	public:
		AnimatedIconManager (IconSetter_t setter, QObject* parent = 0)
		: QObject (parent)
		, Setter_ (setter)
		{
		}

		void SetIcon (const T& t, const QString& name)
		{
			QFile file (name);
			if (!file.open (QIODevice::ReadOnly))
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to open file"
						<< name;
				return;
			}

			SetIcon (t, &file);
		}

		void SetIcon (const T& t, QIODevice *dev)
		{
			Cancel (t);

			if (dev && dev->atEnd ())
				dev->seek (0);
			QImageReader reader (dev);
			const int w = reader.size ().width ();
			const int h = reader.size ().height ();
			if (w == h &&
					reader.imageCount () <= 1)
			{
				Setter_ (t, QIcon (QPixmap::fromImage (reader.read ())));
				return;
			}

			int delay = 0;
			QList<QImage> images;
			if (reader.supportsAnimation ())
			{
				QImage image = reader.read ();
				while (!image.isNull ())
				{
					images << image;
					image = reader.read ();
				}

				delay = reader.nextImageDelay ();
			}
			else if (!(w % h))
			{
				QImage image = reader.read ();
				int frame = 0;
				while (frame * h < w)
				{
					images << image.copy (frame * h, 0, h, h);
					++frame;
				}

				delay = 200;
			}

			IconInfo info =
			{
				0,
				images,
				startTimer (delay)
			};

			Setter_ (t, QIcon (QPixmap::fromImage (images.first ())));

			Object2Icon_ [t] = info;
			Timer2Object_ [info.TimerID_] = t;
		}

		void Cancel (const T& t)
		{
			if (!Object2Icon_.contains (t))
				return;

			const int timerId = Object2Icon_.take (t).TimerID_;
			killTimer (timerId);
			Timer2Object_.remove (timerId);
		}
	protected:
		void timerEvent (QTimerEvent *e)
		{
			QObject::timerEvent (e);

			const int id = e->timerId ();

			const T& t = Timer2Object_ [id];
			IconInfo info = Object2Icon_ [t];
			if (++info.CurrentFrame_ == info.Images_.size ())
				info.CurrentFrame_ = 0;
			Setter_ (t, QIcon (QPixmap::fromImage (info.Images_ [info.CurrentFrame_])));

			Object2Icon_ [t] = info;
		}
	};
}
}

#endif
