/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QDir>
#include "nativeplaylist.h"

namespace LC
{
namespace LMP
{
	class StaticPlaylistManager : public QObject
	{
		Q_OBJECT

		QDir PlaylistsDir_;
	public:
		StaticPlaylistManager (QObject* = 0);

		void SetOnLoadPlaylist (const NativePlaylist_t&);
		NativePlaylist_t GetOnLoadPlaylist () const;

		void SaveCustomPlaylist (QString, const NativePlaylist_t&);

		QStringList EnumerateCustomPlaylists () const;
		NativePlaylist_t GetCustomPlaylist (const QString&) const;

		QString GetCustomPlaylistPath (const QString&) const;
		void DeleteCustomPlaylist (const QString&);
	private:
		void WritePlaylist (const QString&, const NativePlaylist_t&);
		NativePlaylist_t ReadPlaylist (const QString&) const;
	signals:
		void customPlaylistsChanged ();
	};
}
}
