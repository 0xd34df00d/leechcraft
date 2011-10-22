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
#include <boost/bind.hpp>
#include <QDateTime>
#include <QVariant>
#include <QImage>
#include <QAction>
#include <QtDebug>
#include <util/util.h>
#include "metaaccount.h"
#include "metamessage.h"
#include "managecontactsdialog.h"
#include "core.h"

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
		ActionMCSep_ = Util::CreateSeparator (this);
		ActionManageContacts_ = new QAction (tr ("Manage contacts..."),
				this);
		connect (ActionManageContacts_,
				SIGNAL (triggered ()),
				this,
				SLOT (handleManageContacts ()));
	}

	QObjectList MetaEntry::GetAvailEntryObjs () const
	{
		return AvailableRealEntries_;
	}

	QStringList MetaEntry::GetRealEntries () const
	{
		QStringList result = UnavailableRealEntries_;
		Q_FOREACH (QObject *entryObj, AvailableRealEntries_)
			result << qobject_cast<ICLEntry*> (entryObj)->GetEntryID ();
		return result;
	}

	void MetaEntry::SetRealEntries (const QStringList& ids)
	{
		UnavailableRealEntries_ = ids;
	}

	void MetaEntry::AddRealObject (ICLEntry *entry)
	{
		QObject *entryObj = entry->GetObject ();

		AvailableRealEntries_ << entryObj;
		UnavailableRealEntries_.removeAll (entry->GetEntryID ());

		handleRealVariantsChanged (entry->Variants (), entryObj);
		Q_FOREACH (QObject *object, entry->GetAllMessages ())
			handleRealGotMessage (object);

		emit statusChanged (GetStatus (QString ()), QString ());

		ConnectStandardSignals (entryObj);
		if (qobject_cast<IAdvancedCLEntry*> (entryObj))
			ConnectAdvancedSiganls (entryObj);
	}

	QString MetaEntry::GetMetaVariant (QObject *entry, const QString& realVar) const
	{
		QPair<QObject*, QString> pair = qMakePair (entry, realVar);
		return Variant2RealVariant_.key (pair);
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
		QStringList result;
		Q_FOREACH (QObject *entryObj, AvailableRealEntries_)
		{
			ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);

			const QString& name = entry->GetEntryName ();
			QStringList variants = entry->Variants ();
			if (!variants.contains (QString ()))
				variants.prepend (QString ());
			Q_FOREACH (const QString& var, variants)
			{
				const QString& full = name + '/' + var;
				if (!Variant2RealVariant_.contains (full))
				{
					qWarning () << Q_FUNC_INFO
							<< "skipping out-of-sync with variant map:"
							<< name
							<< var
							<< Variant2RealVariant_;
					continue;
				}

				result << full;
			}
		}
		return result;
	}

	QObject* MetaEntry::CreateMessage (IMessage::MessageType type, const QString& variant, const QString& body)
	{
		return ActWithVariant<QObject*, ICLEntry*> (boost::bind (&ICLEntry::CreateMessage,
					_1,
					type,
					_2,
					body),
				variant);
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
		return Messages_;
	}

	void MetaEntry::PurgeMessages (const QDateTime& from)
	{
		Q_FOREACH (QObject *obj, AvailableRealEntries_)
			qobject_cast<ICLEntry*> (obj)->PurgeMessages (from);
	}

	void MetaEntry::SetChatPartState (ChatPartState state, const QString& variant)
	{
		ActWithVariant<void, ICLEntry*> (boost::bind (&ICLEntry::SetChatPartState,
					_1,
					state,
					_2),
				variant);
	}

	EntryStatus MetaEntry::GetStatus (const QString& variant) const
	{
		return ActWithVariant<EntryStatus, ICLEntry*> (boost::bind (&ICLEntry::GetStatus,
					_1,
					_2),
				variant);
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

		if (!result.isEmpty ())
			result << ActionMCSep_;

		result << ActionManageContacts_;

		return result;
	}

	QMap<QString, QVariant> MetaEntry::GetClientInfo (const QString& variant) const
	{
		return ActWithVariant<QMap<QString, QVariant>, ICLEntry*> (boost::bind (&ICLEntry::GetClientInfo,
					_1,
					_2),
				variant);
	}

	void MetaEntry::MarkMsgsRead ()
	{
	}

	IAdvancedCLEntry::AdvancedFeatures MetaEntry::GetAdvancedFeatures () const
	{
		return AFSupportsAttention;
	}

	void MetaEntry::DrawAttention (const QString& text, const QString& variant)
	{
		ActWithVariant<void, IAdvancedCLEntry*> (boost::bind (&IAdvancedCLEntry::DrawAttention,
					_1,
					text,
					_2),
				variant);
	}

	template<typename T, typename U>
	T MetaEntry::ActWithVariant (boost::function<T (U, const QString&)> func, const QString& variant) const
	{
		if (variant.isEmpty ())
		{
			if (AvailableRealEntries_.size ())
				return func (qobject_cast<U> (AvailableRealEntries_.first ()), QString ());
			else
				return T ();
		}

		if (!Variant2RealVariant_.contains (variant))
		{
			qWarning () << Q_FUNC_INFO
					<< variant
					<< "doesn't exist";
			return T ();
		}

		const QPair<QObject*, QString>& pair = Variant2RealVariant_ [variant];
		return func (qobject_cast<U> (pair.first), pair.second);
	}

	void MetaEntry::ConnectStandardSignals (QObject *entryObj)
	{
		connect (entryObj,
				SIGNAL (gotMessage (QObject*)),
				this,
				SLOT (handleRealGotMessage (QObject*)));
		connect (entryObj,
				SIGNAL (statusChanged (const EntryStatus&, const QString&)),
				this,
				SLOT (handleRealStatusChanged (const EntryStatus&, const QString&)));
		connect (entryObj,
				SIGNAL (availableVariantsChanged (const QStringList&)),
				this,
				SLOT (handleRealVariantsChanged (const QStringList&)));
		connect (entryObj,
				SIGNAL (nameChanged (const QString&)),
				this,
				SLOT (handleRealNameChanged (const QString&)));
		connect (entryObj,
				SIGNAL (chatPartStateChanged (const ChatPartState&, const QString&)),
				this,
				SLOT (handleRealCPSChanged (const ChatPartState&, const QString&)));
		connect (entryObj,
				SIGNAL (entryGenerallyChanged ()),
				this,
				SIGNAL (entryGenerallyChanged ()));

		ICLEntry *entry = qobject_cast<ICLEntry*> (entryObj);
		connect (entry->GetParentAccount (),
				SIGNAL (removedCLItems (QList<QObject*>)),
				this,
				SLOT (checkRemovedCLItems (QList<QObject*>)));
	}

	void MetaEntry::ConnectAdvancedSiganls (QObject *entryObj)
	{
		connect (entryObj,
				SIGNAL (attentionDrawn (const QString&, const QString&)),
				this,
				SLOT (handleRealAttentionDrawn (const QString&, const QString&)));
		connect (entryObj,
				SIGNAL (moodChanged (const QString&)),
				this,
				SLOT (handleRealMoodChanged (const QString&)));
		connect (entryObj,
				SIGNAL (activityChanged (const QString&)),
				this,
				SLOT (handleRealActivityChanged (const QString&)));
		connect (entryObj,
				SIGNAL (tuneChanged (const QString&)),
				this,
				SLOT (handleRealTuneChanged (const QString&)));
		connect (entryObj,
				SIGNAL (locationChanged (const QString&)),
				this,
				SLOT (handleRealLocationChanged (const QString&)));
	}

	void MetaEntry::PerformRemoval (QObject *entryObj)
	{
		QObjectList::iterator i = Messages_.begin ();
		while (i < Messages_.end ())
		{
			MetaMessage *metaMsg = qobject_cast<MetaMessage*> (*i);
			IMessage *origMsg = metaMsg->GetOriginalMessage ();

			if (origMsg->OtherPart () == entryObj)
				i = Messages_.erase (i);
			else
				++i;
		}

		Q_FOREACH (const QString& var, Variant2RealVariant_.keys ())
		{
			const QPair<QObject*, QString>& pair = Variant2RealVariant_ [var];
			if (pair.first == entryObj)
			{
				Variant2RealVariant_.remove (var);
				emit statusChanged (EntryStatus (SOffline, QString ()), var);
			}
		}

		emit availableVariantsChanged (Variants ());
	}

	void MetaEntry::SetNewEntryList (const QList<QObject*>& newList)
	{
		if (newList == AvailableRealEntries_)
			return;

		QList<QObject*> removedContacts;

		Q_FOREACH (QObject *obj, AvailableRealEntries_)
			if (!newList.contains (obj))
				removedContacts << obj;

		AvailableRealEntries_ = newList;

		Q_FOREACH (QObject *entryObj, removedContacts)
			PerformRemoval (entryObj);

		emit entriesRemoved (removedContacts);

		if (AvailableRealEntries_.isEmpty () &&
				UnavailableRealEntries_.isEmpty ())
		{
			emit shouldRemoveThis ();
			return;
		}

		emit availableVariantsChanged (Variants ());
		emit statusChanged (GetStatus (QString ()), QString ());
	}

	void MetaEntry::handleRealGotMessage (QObject *msgObj)
	{
		IMessage *msg = qobject_cast<IMessage*> (msgObj);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< msgObj
					<< "doesn't implement IMessage";
			return;
		}

		MetaMessage *message = new MetaMessage (msgObj, this);

		const bool shouldSort = !Messages_.isEmpty () &&
				qobject_cast<IMessage*> (Messages_.last ())->GetDateTime () > msg->GetDateTime ();

		Messages_ << message;
		if (shouldSort)
			std::stable_sort (Messages_.begin (), Messages_.end (), DateSorter ());

		emit gotMessage (message);
	}

	void MetaEntry::handleRealStatusChanged (const EntryStatus& status, const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit statusChanged (status, entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealVariantsChanged (QStringList variants, QObject *passedObj)
	{
		QObject *obj = passedObj ? passedObj : sender ();
		Q_FOREACH (const QString& var, Variant2RealVariant_.keys ())
		{
			const QPair<QObject*, QString>& pair = Variant2RealVariant_ [var];
			if (pair.first == obj)
				Variant2RealVariant_.remove (var);
		}

		ICLEntry *entry = qobject_cast<ICLEntry*> (obj);

		if (!variants.contains (QString ()))
			variants.prepend (QString ());

		Q_FOREACH (const QString& var, variants)
			Variant2RealVariant_ [entry->GetEntryName () + '/' + var] =
					qMakePair (obj, var);

		emit availableVariantsChanged (Variants ());

		Q_FOREACH (const QString& var, variants)
		{
			const QString& str = entry->GetEntryName () + '/' + var;
			emit statusChanged (GetStatus (str), str);
		}
	}

	void MetaEntry::handleRealNameChanged (const QString& name)
	{
		QObject *obj = sender ();
		ICLEntry *entry = qobject_cast<ICLEntry*> (obj);

		handleRealVariantsChanged (entry->Variants (), obj);
	}

	void MetaEntry::handleRealCPSChanged (const ChatPartState& cps, const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit chatPartStateChanged (cps, entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealAttentionDrawn (const QString& text, const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit attentionDrawn (text, entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealMoodChanged (const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit moodChanged (entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealActivityChanged (const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit activityChanged (entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealTuneChanged (const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit tuneChanged (entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::handleRealLocationChanged (const QString& var)
	{
		ICLEntry *entry = qobject_cast<ICLEntry*> (sender ());
		emit locationChanged (entry->GetEntryName () + '/' + var);
	}

	void MetaEntry::checkRemovedCLItems (const QList<QObject*>& objs)
	{
		QList<QObject*> leave = AvailableRealEntries_;
		Q_FOREACH (QObject *obj, objs)
			leave.removeAll (obj);

		if (leave.size () != AvailableRealEntries_.size ())
			SetNewEntryList (leave);
	}

	void MetaEntry::handleManageContacts ()
	{
		ManageContactsDialog dia (AvailableRealEntries_);
		if (dia.exec () == QDialog::Rejected)
			return;

		SetNewEntryList (dia.GetObjects ());
	}
}
}
}
