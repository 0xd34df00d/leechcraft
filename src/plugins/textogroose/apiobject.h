/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QVariantMap>
#include <interfaces/iscriptloader.h>
#include <interfaces/media/ilyricsfinder.h>

namespace LC
{
namespace Textogroose
{
	class ApiObject : public QObject
	{
		Q_OBJECT

		const Media::LyricsQuery Query_;
		const IScript_ptr Script_;
	public:
		ApiObject (const Media::LyricsQuery&, IScript_ptr);
	public slots:
		void handleFinished (const QVariantMap&);
		void handleFinished (const QVariantList&);
	signals:
		void finished (ApiObject*, const Media::LyricsResults&);
	};
}
}
