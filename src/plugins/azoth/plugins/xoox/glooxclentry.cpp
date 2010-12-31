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

	GlooxCLEntry::GlooxCLEntry (GlooxCLEntry::OfflineDataSource_ptr ods, GlooxAccount *parent)
	: EntryBase (parent)
	, ParentAccountObject_ (parent)
	, RI_ (0)
	, ODS_ (ods)
	{
	}

	void GlooxCLEntry::UpdateRI (gloox::RosterItem *ri)
	{
		RI_ = ri;
		ODS_.reset ();

		emit availableVariantsChanged (Variants ());
	}

	gloox::RosterItem* GlooxCLEntry::GetRI () const
	{
		return RI_;
	}

	QList<const gloox::Resource*> GlooxCLEntry::GetResourcesDesc () const
	{
		QList<const gloox::Resource*> result;
		Q_FOREACH (const std::string& str, VariantsImpl ())
			result << RI_->resource (str);
		return result;
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
		if (ODS_)
			return ODS_->Name_;

		std::string name = RI_->name ();
		if (!name.size ())
			name = RI_->jid ();
		return QString::fromUtf8 (name.c_str ());
	}

	void GlooxCLEntry::SetEntryName (const QString& name)
	{
		if (ODS_)
			return;

		RI_->setName (name.toUtf8 ().constData ());
		ParentAccountObject_->Synchronize ();
	}

	QByteArray GlooxCLEntry::GetEntryID () const
	{
		if (ODS_)
			return ODS_->ID_;

		return RI_->jid ().c_str ();
	}

	QStringList GlooxCLEntry::Groups () const
	{
		if (ODS_)
			return ODS_->Groups_;

		QStringList result;
		Q_FOREACH (const std::string& string, RI_->groups ())
			result << QString::fromUtf8 (string.c_str ());
		return result;
	}

	QList<std::string> GlooxCLEntry::VariantsImpl () const
	{
		QMap<int, std::string> prio2resource;
		std::pair<std::string, gloox::Resource*> pair;
		Q_FOREACH (pair, RI_->resources ())
			prio2resource [pair.second->priority ()] = pair.first;

		QList<std::string> result;

		QList<int> keys = prio2resource.keys ();
		if (keys.size ())
		{
			std::sort (keys.begin (), keys.end ());
			std::reverse (keys.begin (), keys.end ());
			Q_FOREACH (int key, keys)
				result << prio2resource [key];
		}

		return result;
	}

	QStringList GlooxCLEntry::Variants () const
	{
		QStringList result;

		if (ODS_)
			return result;

		Q_FOREACH (const std::string& str, VariantsImpl ())
			result << QString::fromUtf8 (str.c_str ());

		if (result.isEmpty ())
			result << QString ();

		return result;
	}

	QObject* GlooxCLEntry::CreateMessage (IMessage::MessageType type,
			const QString& variant, const QString& text)
	{
		if (ODS_)
		{
			// TODO
			return 0;
		}

		QObject *msg = ParentAccountObject_->CreateMessage (type, variant, text, RI_);
		AllMessages_ << msg;
		return msg;
	}
}
}
}
}
}
