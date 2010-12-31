/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "entrybase.h"
#include <QImage>
#include <QStringList>
#include <QtDebug>
#include <gloox/rosteritem.h>
#include "glooxmessage.h"
#include "glooxclentry.h"

namespace LeechCraft
{
namespace Plugins
{
namespace Azoth
{
namespace Plugins
{
namespace Xoox
{
	EntryBase::EntryBase (QObject* parent)
	: QObject (parent)
	{

	}

	QObject* EntryBase::GetObject ()
	{
		return this;
	}


	QList<QObject*> EntryBase::GetAllMessages () const
	{
		return AllMessages_;
	}

	EntryStatus EntryBase::GetStatus (const QString& variant) const
	{
		const GlooxCLEntry *entry = qobject_cast<const GlooxCLEntry*> (this);
		const gloox::Resource *hr = 0;
		if (entry && entry->GetRI ())
			hr = entry->GetRI ()->highestResource ();
		if (CurrentStatus_.contains (variant))
			return CurrentStatus_ [variant];
		else if (hr)
			return EntryStatus (static_cast<State> (hr->presence ()),
					QString::fromUtf8 (hr->message ().c_str ()));
		else if (CurrentStatus_.size ())
			return *CurrentStatus_.begin ();
		else
			return EntryStatus ();
	}

	QList<QAction*> EntryBase::GetActions () const
	{
		return Actions_;
	}

	QImage EntryBase::GetAvatar () const
	{
		return Avatar_;
	}

	void EntryBase::HandleMessage (GlooxMessage *msg)
	{
		AllMessages_ << msg;
		emit gotMessage (msg);
	}

	void EntryBase::SetStatus (const EntryStatus& status, const QString& variant)
	{
		if (status == CurrentStatus_ [variant])
			return;

		CurrentStatus_ [variant] = status;
		emit statusChanged (status, variant);
	}

	void EntryBase::SetPhoto (const gloox::VCard::Photo& photo)
	{
		if (!photo.type.size ())
			Avatar_ = QImage ();
		else
		{
			QByteArray data (photo.binval.c_str(), photo.binval.size ());
			Avatar_ = QImage::fromData (data);
		}

		emit avatarChanged (Avatar_);
	}
}
}
}
}
}
