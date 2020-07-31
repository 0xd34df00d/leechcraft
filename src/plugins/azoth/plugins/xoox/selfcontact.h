/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include "entrybase.h"

namespace LC
{
namespace Azoth
{
namespace Xoox
{
	class SelfContact : public EntryBase
	{
		Q_OBJECT

		QString BareJID_;
		QString Resource_;

		QMap<QString, int> Status2Prio_;
	public:
		SelfContact (const QString&, GlooxAccount*);

		Features GetEntryFeatures () const override;
		EntryType GetEntryType () const override;
		QString GetEntryName () const override;
		void SetEntryName (const QString&) override;
		QString GetEntryID () const override;
		QStringList Groups () const override;
		void SetGroups (const QStringList&) override;
		QStringList Variants () const override;
		EntryStatus GetStatus (const QString&) const override;
		IMessage* CreateMessage (IMessage::Type,
				const QString&, const QString&) override;
		QList<QAction*> GetActions () const override;

		void HandlePresence (const QXmppPresence&, const QString&) override;

		void RemoveVariant (const QString&, bool);
		QString GetJID () const override;
		void UpdateJID (const QString&);
	private:
		void UpdatePriority (const QString&, int);
	private slots:
		void handleSelfVCardUpdated ();
	};
}
}
}
