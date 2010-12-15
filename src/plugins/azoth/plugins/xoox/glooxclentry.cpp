/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "glooxclentry.h"
#include <boost/bind.hpp>
#include <QStringList>
#include <QtDebug>
#include <gloox/rosteritem.h>
#include <gloox/resource.h>
#include <interfaces/iaccount.h>
#include <interfaces/azothcommon.h>
#include "glooxaccount.h"

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
	GlooxCLEntry::GlooxCLEntry (gloox::RosterItem *ri, GlooxAccount *parent)
	: EntryBase (parent)
	, ParentAccountObject_ (parent)
	, RI_ (ri)
	{
	}

	void GlooxCLEntry::UpdateRI (gloox::RosterItem *ri)
	{
		RI_ = ri;
	}

	QObject* GlooxCLEntry::GetParentAccount () const
	{
		return ParentAccountObject_;
	}

	ICLEntry::Features GlooxCLEntry::GetEntryFeatures () const
	{
		return FPermanentEntry | FSupportsRenames;
	}

	ICLEntry::EntryType GlooxCLEntry::GetEntryType () const
	{
		return ETChat;
	}

	QString GlooxCLEntry::GetEntryName () const
	{
		std::string name = RI_->name ();
		if (!name.size ())
			name = RI_->jid ();
		return QString::fromUtf8 (name.c_str ());
	}

	void GlooxCLEntry::SetEntryName (const QString& name)
	{
		RI_->setName (name.toUtf8 ().constData ());
		ParentAccountObject_->Synchronize ();
	}

	QByteArray GlooxCLEntry::GetEntryID () const
	{
		return RI_->jid ().c_str ();
	}

	QStringList GlooxCLEntry::Groups () const
	{
		QStringList result;
		Q_FOREACH (const std::string& string, RI_->groups ())
			result << QString::fromUtf8 (string.c_str ());
		return result;
	}

	QStringList GlooxCLEntry::Variants () const
	{
		QStringList result;

		QMap<int, QString> prio2resource;
		std::pair<std::string, gloox::Resource*> pair;
		Q_FOREACH (pair, RI_->resources ())
		{
			std::string origStr = pair.first;
			int prio = pair.second->priority ();

			prio2resource [prio] = QString::fromUtf8 (origStr.c_str ());
		}

		QList<int> keys = prio2resource.keys ();
		if (keys.size ())
		{
			std::sort (keys.begin (), keys.end ());
			std::reverse (keys.begin (), keys.end ());
			Q_FOREACH (int key, keys)
				result << prio2resource [key];
		}

		if (result.isEmpty ())
			result << "";

		return result;
	}

	QObject* GlooxCLEntry::CreateMessage (IMessage::MessageType type,
			const QString& variant, const QString& text)
	{
		QObject *msg = ParentAccountObject_->CreateMessage (type, variant, text, RI_);
		AllMessages_ << msg;
		return msg;
	}
}
}
}
}
}
