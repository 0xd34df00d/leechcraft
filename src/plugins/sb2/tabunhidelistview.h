/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <interfaces/ihavetabs.h>
#include <interfaces/core/icoreproxy.h>
#include <util/qml/unhidelistviewbase.h>

namespace LC::SB2
{
	class TabUnhideListView : public Util::UnhideListViewBase
	{
		Q_OBJECT
	public:
		TabUnhideListView (const QList<TabClassInfo>&, ICoreProxy_ptr, QWidget* = nullptr);
	private slots:
		void unhide (const QString&);
	signals:
		void unhideRequested (const QByteArray&);
	};
}
