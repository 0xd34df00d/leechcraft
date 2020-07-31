/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <interfaces/structures.h>
#include <util/xpc/util.h>

namespace LC
{
namespace BitTorrent
{
	class NotifyManager : public QObject
	{
		Q_OBJECT

		bool IsReady_;
		QList<Entity> Queue_;
	public:
		NotifyManager (QObject* = 0);

		void PluginsAvailable ();
		void AddNotification (const Entity&);

		template<typename T1, typename T2>
		void AddNotification (T1&& header, T2&& text, Priority p)
		{
			AddNotification (Util::MakeNotification (std::forward<T1> (header), std::forward<T2> (text), p));
		}
	private:
		void SendNotification (const Entity&);
	private slots:
		void makeDelayedReady ();
	};
}
}
