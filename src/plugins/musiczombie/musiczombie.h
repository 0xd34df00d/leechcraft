/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2012  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/iinfo.h>
#include <interfaces/media/idiscographyprovider.h>

#ifdef WITH_CHROMAPRINT
#include <interfaces/media/itagsfetcher.h>
#endif

namespace LC
{
namespace Util
{
	class QueueManager;
}

namespace MusicZombie
{
	class Plugin : public QObject
				 , public IInfo
				 , public Media::IDiscographyProvider
#ifdef WITH_CHROMAPRINT
				 , public Media::ITagsFetcher
#endif
	{
		Q_OBJECT
		Q_INTERFACES (IInfo Media::IDiscographyProvider)
#ifdef WITH_CHROMAPRINT
		Q_INTERFACES (Media::ITagsFetcher)
#endif

		LC_PLUGIN_METADATA ("org.LeechCraft.MusicZombie")

		ICoreProxy_ptr Proxy_;

		Util::QueueManager *Queue_;
		Util::QueueManager *AcoustidQueue_;
	public:
		void Init (ICoreProxy_ptr) override;
		void SecondInit () override;
		QByteArray GetUniqueID () const override;
		void Release () override;
		QString GetName () const override;
		QString GetInfo () const override;
		QIcon GetIcon () const override;

		QString GetServiceName () const override;

		QFuture<Result_t> GetDiscography (const QString&, const QStringList&) override;
		QFuture<Result_t> GetReleaseInfo (const QString&, const QString&) override;

#ifdef WITH_CHROMAPRINT
		QFuture<Media::AudioInfo> FetchTags (const QString&) override;
#endif
	};
}
}
