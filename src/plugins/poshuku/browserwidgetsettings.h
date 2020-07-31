/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QTime>
#include <QPoint>
#include <QMetaType>

class QDataStream;

namespace LC
{
namespace Poshuku
{
	struct BrowserWidgetSettings
	{
		qreal ZoomFactor_;
		bool NotifyWhenFinished_;
		QTime ReloadInterval_;
		QByteArray WebHistorySerialized_;
		QPoint ScrollPosition_;
		QString DefaultEncoding_;
	};

	QDataStream& operator<< (QDataStream&, const BrowserWidgetSettings&);
	QDataStream& operator>> (QDataStream&, BrowserWidgetSettings&);
}
}

Q_DECLARE_METATYPE (LC::Poshuku::BrowserWidgetSettings)
