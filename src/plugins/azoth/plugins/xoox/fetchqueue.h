/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#ifndef PLUGINS_AZOTH_PLUGINS_XOOX_FETCHQUEUE_H
#define PLUGINS_AZOTH_PLUGINS_XOOX_FETCHQUEUE_H
#include <functional>
#include <QObject>
#include <QSet>
#include <QStringList>

class QTimer;

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class FetchQueue : public QObject
	{
		Q_OBJECT

		QTimer *FetchTimer_;
		QStringList Queue_;
		std::function<void (const QString&, bool)> FetchFunction_;
		int PerShot_;
		QSet<QString> Reports_;
	public:
		enum Priority
		{
			PHigh,
			PLow
		};
		FetchQueue (std::function<void (const QString&, bool)> func,
				int timeout, int perShot, QObject* = 0);

		void Schedule (const QString&, Priority = PLow, bool report = false);
		void Clear ();
	private slots:
		void handleFetch ();
	};
}
}
}

#endif
