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

#include "metaentry.h"
#include <algorithm>
#include <QDateTime>
#include <QVariant>
#include <QImage>
#include <QtDebug>
#include "metaaccount.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Metacontacts
{
	MetaEntry::MetaEntry (const QString& id, MetaAccount *account)
	: QObject (account)
	, Account_ (account)
	, ID_ (id)
	{
	}
	
	QObject* MetaEntry::GetObject ()
	{
		return this;
	}

	QObject* MetaEntry::GetParentAccount () const
	{
		return Account_;
	}
	
	ICLEntry::Features MetaEntry::GetEntryFeatures () const
	{
		return FPermanentEntry | FSupportsGrouping | FSupportsRenames;
	}

	ICLEntry::EntryType MetaEntry::GetEntryType () const
	{
		return ETChat;
	}

	QString MetaEntry::GetEntryName () const
	{
		return Name_;
	}

	void MetaEntry::SetEntryName (const QString& name)
	{
		Name_ = name;
		emit nameChanged (name);
	}
	
	QString MetaEntry::GetEntryID () const
	{
		return ID_;
	}
	
	QString MetaEntry::GetHumanReadableID () const
	{
		return GetEntryName () + "@metacontact";
	}
	
	QStringList MetaEntry::Groups () const
	{
		return Groups_;
	}
	
	void MetaEntry::SetGroups (const QStringList& groups)
	{
		Groups_ = groups;
		emit groupsChanged (groups);
	}
	
	QStringList MetaEntry::Variants () const
	{
		return Variant2RealVariant_.keys ();
	}
	
	QObject* MetaEntry::CreateMessage (IMessage::MessageType type, const QString& variant, const QString& body)
	{
		if (variant.isEmpty ())
		{
			if (AvailableRealEntries_.size ())
				return qobject_cast<ICLEntry*> (AvailableRealEntries_.first ())->
						CreateMessage (type, QString (), body);
			else
				return 0;
		}

		if (!Variant2RealVariant_.contains (variant))
		{
			qWarning () << Q_FUNC_INFO
					<< variant
					<< "doesn't exist";
			return 0;
		}
		
		const QPair<QObject*, QString>& pair = Variant2RealVariant_ [variant];
		return qobject_cast<ICLEntry*> (pair.first)->
				CreateMessage (type, pair.second, body);
	}
	
	namespace
	{
		struct DateSorter
		{
			bool operator() (QObject *lObj, QObject *rObj) const
			{
				IMessage *left = qobject_cast<IMessage*> (lObj);
				IMessage *right = qobject_cast<IMessage*> (rObj);
				
				return left->GetDateTime () < right->GetDateTime ();
			}
		};
	}
	
	QList<QObject*> MetaEntry::GetAllMessages () const
	{
		QList<QObject*> result;
		Q_FOREACH (QObject *obj, AvailableRealEntries_)
			result << qobject_cast<ICLEntry*> (obj)->GetAllMessages ();
		
		std::stable_sort (result.begin (), result.end (), DateSorter ());
		return result;
	}
	
	void MetaEntry::PurgeMessages (const QDateTime& from)
	{
		Q_FOREACH (QObject *obj, AvailableRealEntries_)
			qobject_cast<ICLEntry*> (obj)->PurgeMessages (from);
	}
	
	void MetaEntry::SetChatPartState (ChatPartState state, const QString& variant)
	{
		if (!Variant2RealVariant_.contains (variant))
		{
			qWarning () << Q_FUNC_INFO
					<< variant
					<< "doesn't exist";
			return;
		}
		
		const QPair<QObject*, QString>& pair = Variant2RealVariant_ [variant];
		qobject_cast<ICLEntry*> (pair.first)->SetChatPartState (state, pair.second);
	}
	
	EntryStatus MetaEntry::GetStatus (const QString& variant) const
	{
		if (variant.isEmpty ())
		{
			if (AvailableRealEntries_.size ())
				return qobject_cast<ICLEntry*> (AvailableRealEntries_.first ())->GetStatus ();
			else
				return EntryStatus ();
		}
		
		if (!Variant2RealVariant_.contains (variant))
		{
			qWarning () << Q_FUNC_INFO
					<< variant
					<< "doesn't exist";
			return EntryStatus ();
		}
		
		const QPair<QObject*, QString>& pair = Variant2RealVariant_ [variant];
		return qobject_cast<ICLEntry*> (pair.first)->GetStatus (pair.second);
	}
	
	QImage MetaEntry::GetAvatar () const
	{
		return QImage ();
	}
	
	QString MetaEntry::GetRawInfo () const
	{
		return QString ();
	}
	
	void MetaEntry::ShowInfo ()
	{
	}
	
	QList<QAction*> MetaEntry::GetActions () const
	{
		QList<QAction*> result;
		Q_FOREACH (QObject *entryObj, AvailableRealEntries_)
			result << qobject_cast<ICLEntry*> (entryObj)->GetActions ();
		return result;
	}
	
	QMap<QString, QVariant> MetaEntry::GetClientInfo (const QString& variant) const
	{
		if (variant.isEmpty ())
		{
			if (AvailableRealEntries_.size ())
				return qobject_cast<ICLEntry*> (AvailableRealEntries_.first ())->GetClientInfo (QString ());
			else
				return QMap<QString, QVariant> ();
		}

		if (!Variant2RealVariant_.contains (variant))
		{
			qWarning () << Q_FUNC_INFO
					<< variant
					<< "doesn't exist";
			return QMap<QString, QVariant> ();
		}
		
		const QPair<QObject*, QString>& pair = Variant2RealVariant_ [variant];
		return qobject_cast<ICLEntry*> (pair.first)->GetClientInfo (pair.second);
	}
}
}
}
