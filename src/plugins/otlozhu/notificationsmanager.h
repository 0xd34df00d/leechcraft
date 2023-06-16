/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "todoitem.h"

class QTimer;

namespace LC
{
struct Entity;

namespace Otlozhu
{
	class TodoStorage;

	class NotificationsManager : public QObject
	{
		Q_OBJECT

		TodoStorage *Storage_;
		QTimer *NextEventTimer_;
		TodoItem_ptr NextEvent_;
	public:
		NotificationsManager (TodoStorage*);
	private slots:
		void handleTimer ();
		void readjustTimer ();
	};
}
}
