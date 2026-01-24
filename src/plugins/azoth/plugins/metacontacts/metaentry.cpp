/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "metaentry.h"
#include <algorithm>
#include <QDateTime>
#include <QVariant>
#include <QImage>
#include <QAction>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sll/prelude.h>
#include <util/util.h>
#include "metaaccount.h"
#include "metamessage.h"
#include "managecontactsdialog.h"
#include "core.h"

namespace LC
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
		return UnavailableRealEntries_ +
				Util::Map (AvailableRealEntries_,
						[] (QObject *entryObj) { return qobject_cast<ICLEntry*> (entryObj)->GetEntryID (); });
	}

	void MetaEntry::SetRealEntries (const QStringList& ids)
	{
		UnavailableRealEntries_ = ids;
	}

	void MetaEntry::AddRealObject (ICLEntry *entry)
	{
		QObject *entryObj = entry->GetQObject ();

		AvailableRealEntries_ << entryObj;
		UnavailableRealEntries_.removeAll (entry->GetEntryID ());

		handleRealVariantsChanged (entry->Variants (), entryObj);
		for (const auto msg : entry->GetAllMessages ())
			handleRealGotMessage (msg->GetQObject ());

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

	QObject* MetaEntry::GetQObject ()
	{
		return this;
	}

	IAccount* MetaEntry::GetParentAccount () const
	{
		return Account_;
	}

	ICLEntry::Features MetaEntry::GetEntryFeatures () const
	{
		return FPermanentEntry | FSupportsGrouping | FSupportsRenames;
	}

	ICLEntry::EntryType MetaEntry::GetEntryType () const
	{
		return EntryType::Chat;
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
		for (const auto entryObj : AvailableRealEntries_)
		{
			const auto entry = qobject_cast<ICLEntry*> (entryObj);

			const auto& name = entry->GetEntryName ();
			auto variants = entry->Variants ();
			if (!variants.contains (QString ()))
				variants.prepend (QString ());
			for (const auto& var : variants)
			{
				const auto& full = name + '/' + var;
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

	template<typename F>
	auto MetaEntry::ActWithVariant (F&& func, const QString& variant) const
	{
		using R = Util::RetType_t<std::decay_t<F>>;
		using Arg = Util::ArgType_t<std::decay_t<F>, 0>;

		if (variant.isEmpty ())
		{
			if (!AvailableRealEntries_.isEmpty ())
				return func (qobject_cast<Arg> (AvailableRealEntries_.first ()), QString ());
			return R ();
		}

		if (!Variant2RealVariant_.contains (variant))
		{
			qWarning () << variant << "doesn't exist";
			return R ();
		}

		const auto& [realEntry, realVar] = Variant2RealVariant_ [variant];
		return func (qobject_cast<Arg> (realEntry), realVar);
	}


	void MetaEntry::SendMessage (const OutgoingMessage& message)
	{
		auto f = [message = message] (ICLEntry *e, const QString& v) mutable
		{
			message.Variant_ = v;
			e->SendMessage (message);
		};
		ActWithVariant (f, message.Variant_.value_or ({}));
	}

	QList<IMessage*> MetaEntry::GetAllMessages () const
	{
		return Messages_;
	}

	void MetaEntry::PurgeMessages (const QDateTime& from)
	{
		for (const auto obj : AvailableRealEntries_)
			qobject_cast<ICLEntry*> (obj)->PurgeMessages (from);
	}

	void MetaEntry::SetChatPartState (ChatPartState state, const QString& variant)
	{
		auto f = [state] (ICLEntry *e, const QString& v) { e->SetChatPartState (state, v); };
		ActWithVariant (f, variant);
	}

	EntryStatus MetaEntry::GetStatus (const QString& variant) const
	{
		auto f = [] (ICLEntry *e, const QString& v) { return e->GetStatus (v); };
		return ActWithVariant (f, variant);
	}

	void MetaEntry::ShowInfo ()
	{
	}

	QList<QAction*> MetaEntry::GetActions () const
	{
		auto result = Util::ConcatMap (AvailableRealEntries_,
				[] (QObject *entryObj) { return qobject_cast<ICLEntry*> (entryObj)->GetActions (); });

		if (!result.isEmpty ())
			result << ActionMCSep_;

		result << ActionManageContacts_;

		return result;
	}

	QMap<QString, QVariant> MetaEntry::GetClientInfo (const QString& variant) const
	{
		auto f = [] (ICLEntry *e, const QString& v) { return e->GetClientInfo (v); };
		return ActWithVariant (f, variant);
	}

	void MetaEntry::MarkMsgsRead ()
	{
	}

	void MetaEntry::ChatTabClosed ()
	{
	}

	IAdvancedCLEntry::AdvancedFeatures MetaEntry::GetAdvancedFeatures () const
	{
		return AFSupportsAttention;
	}

	void MetaEntry::DrawAttention (const QString& text, const QString& variant)
	{
		auto f = [text] (IAdvancedCLEntry *e, const QString& v) { e->DrawAttention (text, v); };
		ActWithVariant (f, variant);
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

		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		connect (entry->GetParentAccount ()->GetQObject (),
				SIGNAL (removedCLItems (QList<QObject*>)),
				this,
				SLOT (checkRemovedCLItems (QList<QObject*>)));

		/*
		IAccount *account = qobject_cast<IAccount*> (entry->GetParentAccount ());
		connect (account->GetParentProtocol (),
				SIGNAL (accountRemoved (QObject*)),
				this,
				SLOT (handleAccountRemoved (QObject*)),
				Qt::QueuedConnection);
				*/
	}

	void MetaEntry::ConnectAdvancedSiganls (QObject *entryObj)
	{
		connect (entryObj,
				SIGNAL (attentionDrawn (const QString&, const QString&)),
				this,
				SLOT (handleRealAttentionDrawn (const QString&, const QString&)));
	}

	void MetaEntry::PerformRemoval (QObject *entryObj)
	{
		for (auto i = Messages_.begin (); i != Messages_.end (); )
		{
			const auto metaMsg = dynamic_cast<MetaMessage*> (*i);
			const auto origMsg = metaMsg->GetOriginalMessage ();

			if (origMsg->OtherPart () == entryObj)
				i = Messages_.erase (i);
			else
				++i;
		}

		for (auto i = Variant2RealVariant_.begin (); i != Variant2RealVariant_.end (); )
		{
			const auto& var = i.key ();
			const auto& pair = i.value ();

			if (pair.first == entryObj)
			{
				emit statusChanged ({ SOffline, QString () }, var);
				i = Variant2RealVariant_.erase (i);
			}
			else
				++i;
		}

		emit availableVariantsChanged (Variants ());
	}

	void MetaEntry::SetNewEntryList (const QList<QObject*>& newList, bool readdRemoved)
	{
		if (newList == AvailableRealEntries_)
			return;

		const auto removedContacts = Util::Filter (AvailableRealEntries_,
				[&newList] (QObject *obj) { return !newList.contains (obj); });

		AvailableRealEntries_ = newList;

		for (const auto entryObj : removedContacts)
			PerformRemoval (entryObj);

		Core::Instance ().HandleEntriesRemoved (removedContacts, readdRemoved);

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
				Messages_.last ()->GetDateTime () > msg->GetDateTime ();

		Messages_ << message;
		if (shouldSort)
			std::stable_sort (Messages_.begin (), Messages_.end (), Util::ComparingBy (&IMessage::GetDateTime));

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
		for (auto it = Variant2RealVariant_.begin (); it != Variant2RealVariant_.end ();)
		{
			if (it.value ().first == obj)
				it = Variant2RealVariant_.erase (it);
			else
				++it;
		}

		ICLEntry *entry = qobject_cast<ICLEntry*> (obj);

		if (!variants.contains (QString ()))
			variants.prepend (QString ());

		for (const auto& var : variants)
			Variant2RealVariant_ [entry->GetEntryName () + '/' + var] = QPair { obj, var };

		emit availableVariantsChanged (Variants ());

		for (const auto& var : variants)
		{
			const auto& str = entry->GetEntryName () + '/' + var;
			emit statusChanged (GetStatus (str), str);
		}
	}

	void MetaEntry::handleRealNameChanged (const QString&)
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

	void MetaEntry::checkRemovedCLItems (const QList<QObject*>& objs)
	{
		auto leave = AvailableRealEntries_;
		for (const auto obj : objs)
			leave.removeAll (obj);

		if (leave.size () != AvailableRealEntries_.size ())
			SetNewEntryList (leave, false);
	}

	void MetaEntry::handleManageContacts ()
	{
		ManageContactsDialog dia (AvailableRealEntries_);
		if (dia.exec () == QDialog::Rejected)
			return;

		SetNewEntryList (dia.GetObjects (), true);
	}
}
}
}
