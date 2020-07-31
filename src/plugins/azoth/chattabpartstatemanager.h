/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "interfaces/azoth/azothcommon.h"

class QTimer;

namespace LC
{
namespace Azoth
{
	class ChatTab;

	class ChatTabPartStateManager : public QObject
	{
		const QString EntryID_;

		ChatPartState PreviousState_ = CPSNone;
		QString LastVariant_;

		QTimer * const TypeTimer_;
	public:
		ChatTabPartStateManager (ChatTab*);
		~ChatTabPartStateManager ();
	private:
		void SetChatPartState (ChatPartState);

		void HandleVariantChanged (const QString&);
		void HandleComposingText (const QString&);

		ICLEntry* GetEntry () const;
	};
}
}
