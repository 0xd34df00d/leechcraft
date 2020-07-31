/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QStringList>
#include <QTranslator>
#include <interfaces/iinfo.h>
#include <interfaces/ifinder.h>
#include <interfaces/media/ilyricsfinder.h>
#include "searcher.h"

namespace LC
{
namespace DeadLyrics
{
	class DeadLyRicS : public QObject
						, public IInfo
						, public Media::ILyricsFinder
	{
		Q_OBJECT
		Q_INTERFACES (IInfo Media::ILyricsFinder)

		LC_PLUGIN_METADATA ("org.LeechCraft.DeadLyrics")

		ICoreProxy_ptr Proxy_;
		Searchers_t Searchers_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		void Release ();
		QByteArray GetUniqueID () const;
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		QFuture<LyricsQueryResult_t> RequestLyrics (const Media::LyricsQuery&);
	};
}
}
