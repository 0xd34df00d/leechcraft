/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "typingmanager.h"
#include <QTimer>

namespace LC
{
namespace Azoth
{
namespace Vader
{
namespace Proto
{
	TypingManager::TypingManager (QObject *parent)
	: QObject (parent)
	, ExpTimer_ (new QTimer (this))
	, OutTimer_ (new QTimer (this))
	{
		ExpTimer_->setInterval (5000);
		connect (ExpTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (checkExpires ()));

		OutTimer_->setInterval (8000);
		connect (OutTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (sendOut ()));
	}

	void TypingManager::GotNotification (const QString& from)
	{
		if (!LastNotDates_.contains (from))
			emit startedTyping (from);

		if (LastNotDates_.isEmpty ())
			ExpTimer_->start ();

		LastNotDates_ [from] = QDateTime::currentDateTime ();
	}

	void TypingManager::SetTyping (const QString& to, bool isTyping)
	{
		if (!isTyping)
		{
			TypingTo_.remove (to);
			if (TypingTo_.isEmpty ())
				OutTimer_->stop ();
		}
		else
		{
			if (TypingTo_.isEmpty ())
				OutTimer_->start ();

			TypingTo_ << to;
			emit needNotify (to);
		}
	}

	void TypingManager::checkExpires ()
	{
		const auto& cur = QDateTime::currentDateTime ();
		for (const auto& from : LastNotDates_.keys ())
		{
			if (LastNotDates_ [from].secsTo (cur) <= 10)
				continue;

			LastNotDates_.remove (from);
			emit stoppedTyping (from);
		}

		if (LastNotDates_.isEmpty ())
			ExpTimer_->stop ();
	}

	void TypingManager::sendOut ()
	{
		for (const auto& to : TypingTo_)
			emit needNotify (to);
	}
}
}
}
}
