/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "typingmanager.h"
#include <QTimer>

namespace LeechCraft
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
		Q_FOREACH (const QString& from, LastNotDates_.keys ())
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
		Q_FOREACH (const QString& to, TypingTo_)
			emit needNotify (to);
	}
}
}
}
}
