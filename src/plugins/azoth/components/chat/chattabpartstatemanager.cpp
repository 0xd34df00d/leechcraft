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
#include "../../xmlsettingsmanager.h"
#include "chattab.h"

namespace LC::Azoth
{
	ChatTabPartStateManager::ChatTabPartStateManager (ChatTab& tab)
	: QObject { &tab }
	, Tab_ { tab }
	, TypeTimer_ { new QTimer { this } }
	{
		auto setState = [this] (ChatPartState st)
		{
			return [this, st] { SetChatPartState (st); };
		};

		XmlSettingsManager::Instance ().RegisterObject ("TypingPausedTimeout",
				this,
				[this] (int seconds) { TypeTimer_->setInterval (std::chrono::seconds { seconds }); });
		TypeTimer_->callOnTimeout (this, setState (CPSPaused));

		connect (&tab,
				&ChatTab::entryLostCurrent,
				this,
				setState (CPSInactive));
		connect (&tab,
				&ChatTab::entryMadeCurrent,
				this,
				setState (CPSActive));
		connect (&tab,
				&ChatTab::messageSent,
				this,
				setState (CPSActive));

		connect (&tab,
				&ChatTab::composingTextChanged,
				this,
				[this] (const QString& text)
				{
					if (!text.isEmpty ())
					{
						SetChatPartState (CPSComposing);
						TypeTimer_->start ();
					}
				});

		connect (&tab,
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

		TypeTimer_->stop ();
		PreviousState_ = state;

		if (!XmlSettingsManager::Instance ().property ("SendChatStates").toBool ())
			return;

		const auto entry = Tab_.GetCLEntry ();
		if (!entry)
			return;

		if (state != CPSGone ||
				XmlSettingsManager::Instance ().property ("SendEndConversations").toBool ())
			entry->SetChatPartState (state, LastVariant_);
	}

	void ChatTabPartStateManager::HandleVariantChanged (const QString& variant)
	{
		if (LastVariant_.isEmpty () || variant == LastVariant_)
			return;

		LastVariant_ = variant;

		if (!XmlSettingsManager::Instance ().property ("SendChatStates").toBool ())
			return;

		if (const auto entry = Tab_.GetCLEntry ();
			entry && entry->GetStatus (LastVariant_).State_ != SOffline)
			entry->SetChatPartState (CPSActive, LastVariant_);
	}
}
