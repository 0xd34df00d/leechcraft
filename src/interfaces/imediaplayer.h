/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef INTERFACES_IMEDIAPLAYER_H
#define INTERFACES_IMEDIAPLAYER_H
#include <QString>
#include <QWidget>
#include <QUrl>
#include <QtPlugin>

class Q_DECL_EXPORT IVideoWidget
{
public:
	virtual ~IVideoWidget () {}

	virtual void Enqueue (const QUrl& url) = 0;
	virtual void Enqueue (QIODevice*) = 0;
	virtual void Play () = 0;
	virtual void Pause () = 0;
	virtual void Stop () = 0;
	virtual void Clear () = 0;

	virtual QWidget* Widget () = 0;
};

class Q_DECL_EXPORT IMediaPlayer
{
public:
	virtual IVideoWidget* CreateWidget () const = 0;
	virtual IVideoWidget* GetDefaultWidget () const = 0;

	virtual ~IMediaPlayer () {}
};

Q_DECLARE_INTERFACE (IVideoWidget, "org.Deviant.LeechCraft.IVideoWidget/1.0")
Q_DECLARE_INTERFACE (IMediaPlayer, "org.Deviant.LeechCraft.IMediaPlayer/1.0")

#endif

