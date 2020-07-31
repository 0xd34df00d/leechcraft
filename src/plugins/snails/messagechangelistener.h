/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <vmime/net/events.hpp>

namespace LC
{
namespace Snails
{
	class MessageChangeListener : public QObject
								, public vmime::net::events::messageChangedListener
	{
		Q_OBJECT

		bool IsEnabled_ = true;
	public:
		MessageChangeListener (QObject*);

		std::shared_ptr<void> Disable ();
	protected:
		void messageChanged (const vmime::shared_ptr<vmime::net::events::messageChangedEvent>&) override;
	signals:
		void messagesChanged (const QStringList& folder, const QList<size_t>& numbers);
	};
}
}
