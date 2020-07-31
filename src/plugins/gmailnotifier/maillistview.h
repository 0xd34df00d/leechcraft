/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QQuickWidget>
#include <interfaces/core/icoreproxy.h>
#include "convinfo.h"

class QStandardItemModel;

namespace LC
{
namespace GmailNotifier
{
	class MailListView : public QQuickWidget
	{
		Q_OBJECT

		QStandardItemModel *Model_;
	public:
		MailListView (const ConvInfos_t&, ICoreProxy_ptr, QWidget* = 0);
	};
}
}
