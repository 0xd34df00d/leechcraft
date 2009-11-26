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

#ifndef INTERFACES_IMEDIAPLAYER_H
#define INTERFACES_IMEDIAPLAYER_H
#include <QString>
#include <QWidget>
#include <QUrl>
#include <QtPlugin>

class IVideoWidget
{
public:
	virtual ~IVideoWidget () {}

	virtual void Enqueue (const QUrl& url) = 0;
	virtual void Enqueue (QIODevice*) = 0;
	virtual void Play () = 0;
	virtual void Pause () = 0;
	virtual void Stop () = 0;

	virtual QWidget* Widget () = 0;
};

class IMediaPlayer
{
public:
	virtual IVideoWidget* CreateWidget () const = 0;
	virtual IVideoWidget* GetDefaultWidget () const = 0;

	virtual ~IMediaPlayer () {}
};

Q_DECLARE_INTERFACE (IVideoWidget, "org.Deviant.LeechCraft.IVideoWidget/1.0");
Q_DECLARE_INTERFACE (IMediaPlayer, "org.Deviant.LeechCraft.IMediaPlayer/1.0");

#endif

