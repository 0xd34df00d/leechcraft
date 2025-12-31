/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/media/ialbumartprovider.h>

namespace LC::Kovrogruz
{
	class Plugin
		: public QObject
		, public IInfo
		, public Media::IAlbumArtProvider
	{
		Q_OBJECT
		Q_INTERFACES (IInfo Media::IAlbumArtProvider)

		LC_PLUGIN_METADATA ("org.LeechCraft.Kovrogruz")
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		QString GetName () const override;
		QString GetInfo () const override;
		void Release () override;
		QIcon GetIcon () const override;

		Channel_t RequestAlbumArt (const Media::AlbumInfo& album) const override;
	};
}
