/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "fetchqueue.h"
#include <QTimer>
#include <QtDebug>

namespace LeechCraft
{
namespace Azoth
{
namespace Xoox
{
	FetchQueue::FetchQueue (boost::function<void (const QString&)> func,
			int timeout, int perShot, QObject *parent)
	: QObject (parent)
	, FetchTimer_ (new QTimer)
	, FetchFunction_ (func)
	, PerShot_ (perShot)
	{
		FetchTimer_->setInterval (timeout);
		connect (FetchTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (handleFetch ()));
	}
	
	void FetchQueue::Schedule (const QString& string, FetchQueue::Priority prio)
	{
		if (Queue_.contains (string))
			return;
		
		switch (prio)
		{
		case PHigh:
			Queue_.prepend (string);
			break;
		case PLow:
			Queue_.append (string);
			break;
		}

		if (!FetchTimer_->isActive ())
		{
			QTimer::singleShot (100,
					this,
					SLOT (handleFetch ()));
			FetchTimer_->start ();
		}
	}
	
	void FetchQueue::handleFetch ()
	{
		int num = std::min (PerShot_, Queue_.size ());
		while (num--)
			FetchFunction_ (Queue_.takeFirst ());

		if (Queue_.isEmpty () &&
				FetchTimer_->isActive ())
			FetchTimer_->stop ();
	}
}
}
}
