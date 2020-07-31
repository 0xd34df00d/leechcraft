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
#include <interfaces/media/ilyricsfinder.h>

class IScriptLoaderInstance;

namespace LC
{
namespace Textogroose
{
	class ApiObject;

	class Plugin : public QObject
				 , public IInfo
				 , public Media::ILyricsFinder
	{
		Q_OBJECT
		Q_INTERFACES (IInfo Media::ILyricsFinder)

		LC_PLUGIN_METADATA ("org.LeechCraft.Textogroose")

		QList<std::shared_ptr<IScriptLoaderInstance>> Loaders_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QFuture<LyricsQueryResult_t> RequestLyrics (const Media::LyricsQuery&);
	};
}
}
