/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "customchatstylemanager.h"
#include <QSettings>
#include <QCoreApplication>
#include "interfaces/azoth/iaccount.h"

namespace LC
{
namespace Azoth
{
	CustomChatStyleManager::CustomChatStyleManager (QObject *parent)
	: QObject (parent)
	{
	}

	QPair<QString, QString> CustomChatStyleManager::GetForEntry (ICLEntry *entry) const
	{
		if (!entry)
			return {};

		const auto acc = entry->GetParentAccount ();
		return entry->GetEntryType () == ICLEntry::EntryType::MUC ?
				GetMUCStyleForAccount (acc) :
				GetStyleForAccount (acc);
	}

	QPair<QString, QString> CustomChatStyleManager::GetStyleForAccount (IAccount *acc) const
	{
		return GetProps ("Chat", acc);
	}

	QPair<QString, QString> CustomChatStyleManager::GetMUCStyleForAccount (IAccount *acc) const
	{
		return GetProps ("MUC", acc);
	}

	void CustomChatStyleManager::Set (IAccount *acc, Settable settable, const QString& value)
	{
		QSettings settings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Azoth");
		settings.beginGroup ("CustomStyles");
		settings.beginGroup (acc->GetAccountID ());

		QString name;
		switch (settable)
		{
		case Settable::ChatStyle:
			name = "ChatStyle";
			break;
		case Settable::ChatVariant:
			name = "ChatVariant";
			break;
		case Settable::MUCStyle:
			name = "MUCStyle";
			break;
		case Settable::MUCVariant:
			name = "MUCVariant";
			break;
		}

		settings.setValue (name, value);

		settings.endGroup ();
		settings.endGroup ();

		emit accountStyleChanged (acc);
	}

	QPair<QString, QString> CustomChatStyleManager::GetProps (const QString& prefix, IAccount *acc) const
	{
		QSettings settings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Azoth");
		settings.beginGroup ("CustomStyles");
		settings.beginGroup (acc->GetAccountID ());
		const auto& style = settings.value (prefix + "Style").toString ();
		const auto& variant = settings.value (prefix + "Variant").toString ();
		settings.endGroup ();
		settings.endGroup ();

		return { style, variant };
	}
}
}
