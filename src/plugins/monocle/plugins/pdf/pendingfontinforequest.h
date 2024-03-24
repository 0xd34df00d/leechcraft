/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <interfaces/monocle/ihavefontinfo.h>

namespace Poppler
{
	class Document;
}

namespace LC::Monocle::PDF
{
	class PendingFontInfoRequest final : public QObject
									   , public IPendingFontInfoRequest
	{
		Q_OBJECT
		Q_INTERFACES (LC::Monocle::IPendingFontInfoRequest)

		QList<FontInfo> Result_;
	public:
		PendingFontInfoRequest (const std::shared_ptr<Poppler::Document>&);

		QObject* GetQObject () override;
		QList<FontInfo> GetFontInfos () const override;
	signals:
		void ready () override;
	};
}
