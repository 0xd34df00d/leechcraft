/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pendingfontinforequest.h"
#include <QtConcurrentRun>
#include "qt5compat.h"
#include <poppler-qt5.h>
#include <util/threads/futures.h>

namespace LC::Monocle::PDF
{
	PendingFontInfoRequest::PendingFontInfoRequest (const std::shared_ptr<Poppler::Document>& doc)
	{
		Util::Sequence (this, QtConcurrent::run ([doc] { return doc->fonts (); })) >>
				[this] (const QList<Poppler::FontInfo>& items)
				{
					for (const auto& item : items)
						Result_.append ({
								item.name (),
								item.file (),
								item.isEmbedded ()
						});
					emit ready ();
					deleteLater ();
				};
	}

	QObject* PendingFontInfoRequest::GetQObject ()
	{
		return this;
	}

	QList<FontInfo> PendingFontInfoRequest::GetFontInfos () const
	{
		return Result_;
	}
}
