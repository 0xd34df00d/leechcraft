/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "chattabpartstatemanager.h"
#include <QTimer>
#include "interfaces/azoth/iclentry.h"
#include "chattab.h"
#include "xmlsettingsmanager.h"
#include "core.h"

namespace LC
{
namespace Azoth
{
	ChatTabPartStateManager::ChatTabPartStateManager (ChatTab *tab)
	: QObject { tab }
	, EntryID_ { tab->GetEntryID () }
	, TypeTimer_ { new QTimer { this } }
	{
		auto setState = [this] (ChatPartState st)
		{
			return [this, st] { SetChatPartState (st); };
		};

		connect (tab,
				&ChatTab::entryLostCurrent,
				this,
				setState (CPSInactive));
		connect (tab,
				&ChatTab::hookMessageSendRequested,
				this,
				setState (CPSActive));

		connect (tab,
				&ChatTab::composingTextChanged,
				this,
				&ChatTabPartStateManager::HandleComposingText);

		TypeTimer_->setInterval (2000);
		connect (TypeTimer_,
				&QTimer::timeout,
				this,
				setState (CPSPaused));
		connect (tab,
				&ChatTab::entryLostCurrent,
				TypeTimer_,
				&QTimer::stop);

		connect (tab,
				&ChatTab::currentVariantChanged,
				this,
				&ChatTabPartStateManager::HandleVariantChanged);
	}

	ChatTabPartStateManager::~ChatTabPartStateManager ()
	{
		SetChatPartState (CPSGone);
	}

	void ChatTabPartStateManager::SetChatPartState (ChatPartState state)
	{
		if (state == PreviousState_)
			return;

		if (!XmlSettingsManager::Instance ().property ("SendChatStates").toBool ())
			return;

		auto entry = GetEntry ();
		if (!entry)
			return;

		PreviousState_ = state;

		if (state != CPSGone ||
				XmlSettingsManager::Instance ().property ("SendEndConversations").toBool ())
			entry->SetChatPartState (state, LastVariant_);
	}

	void ChatTabPartStateManager::HandleComposingText (const QString& text)
	{
		TypeTimer_->stop ();

		if (!text.isEmpty ())
		{
			SetChatPartState (CPSComposing);
			TypeTimer_->start ();
		}
	}

	void ChatTabPartStateManager::HandleVariantChanged (const QString& variant)
	{
		if (LastVariant_.isEmpty () || variant == LastVariant_)
			return;

		LastVariant_ = variant;

		if (!XmlSettingsManager::Instance ().property ("SendChatStates").toBool ())
			return;

		auto entry = GetEntry ();
		if (entry && entry->GetStatus (LastVariant_).State_ != SOffline)
			entry->SetChatPartState (CPSInactive, LastVariant_);
	}

	ICLEntry* ChatTabPartStateManager::GetEntry () const
	{
		return qobject_cast<ICLEntry*> (Core::Instance ().GetEntry (EntryID_));
	}
}
}
