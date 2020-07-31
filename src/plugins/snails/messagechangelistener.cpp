/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "messagechangelistener.h"
#include <QStringList>
#include <QtDebug>
#include <vmime/net/folder.hpp>
#include "vmimeconversions.h"

namespace LC
{
namespace Snails
{
	MessageChangeListener::MessageChangeListener (QObject *parent)
	: QObject { parent }
	{
	}

	std::shared_ptr<void> MessageChangeListener::Disable ()
	{
		if (!IsEnabled_)
			return {};

		IsEnabled_ = false;
		return std::shared_ptr<void> (nullptr, [this] (void*) { IsEnabled_ = true; });
	}

	void MessageChangeListener::messageChanged (const vmime::shared_ptr<vmime::net::events::messageChangedEvent>& event)
	{
		if (!IsEnabled_)
			return;

		const auto& folder = event->getFolder ();

		QList<size_t> numsList;
		for (const auto num : event->getNumbers ())
			numsList << num;

		emit messagesChanged (GetFolderPath (folder), numsList);
	}
}
}
