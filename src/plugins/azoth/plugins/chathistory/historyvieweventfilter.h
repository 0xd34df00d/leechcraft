/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>

class QTextBrowser;
class QUrl;

namespace LC
{
namespace Azoth
{
namespace ChatHistory
{
	class HistoryViewEventFilter : public QObject
	{
		Q_OBJECT

		QTextBrowser * const Viewer_;
	public:
		HistoryViewEventFilter (QTextBrowser*);

		bool eventFilter (QObject*, QEvent*) override;
	signals:
		void bgLinkRequested (const QUrl&);
	};
}
}
}
