/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "util.h"
#include <QString>
#include <QWizard>
#include <util/util.h>
#include <interfaces/structures.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "addaccountwizardfirstpage.h"
#include "core.h"

namespace LeechCraft
{
namespace Azoth
{
	void BuildNotification (Entity& e, ICLEntry *other)
	{
		e.Additional_ ["NotificationPixmap"] =
				QVariant::fromValue<QPixmap> (QPixmap::fromImage (other->GetAvatar ()));
		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.Azoth";
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] =
				"org.LC.AdvNotifications.IM";
		e.Additional_ ["org.LC.AdvNotifications.EventID"] =
				"org.LC.Plugins.Azoth.IncomingMessageFrom/" + other->GetEntryID ();

		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList (other->GetEntryName ());

		e.Additional_ ["org.LC.Plugins.Azoth.SourceName"] = other->GetEntryName ();
		e.Additional_ ["org.LC.Plugins.Azoth.SourceID"] = other->GetEntryID ();
		e.Additional_ ["org.LC.Plugins.Azoth.SourceGroups"] = other->Groups ();
	}

	QString GetActivityIconName (const QString& general, const QString& specific)
	{
		return (general + ' ' + specific).trimmed ().replace (' ', '_');
	}

	void InitiateAccountAddition(QWidget *parent)
	{
		QWizard *wizard = new QWizard (parent);
		wizard->setAttribute (Qt::WA_DeleteOnClose);
		wizard->setWindowTitle (QObject::tr ("Add account"));
		wizard->addPage (new AddAccountWizardFirstPage (wizard));

		wizard->show ();
	}

	void AuthorizeEntry (ICLEntry *entry)
	{
		IAccount *account =
				qobject_cast<IAccount*> (entry->GetParentAccount ());
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< "parent account doesn't implement IAccount:"
					<< entry->GetParentAccount ();
			return;
		}
		const QString& id = entry->GetHumanReadableID ();
		account->Authorize (entry->GetObject ());
		account->RequestAuth (id);

		const auto& e = Util::MakeANCancel ("org.LeechCraft.Azoth",
				"org.LC.Plugins.Azoth.AuthRequestFrom/" + entry->GetEntryID ());
		Core::Instance ().SendEntity (e);
	}

	void DenyAuthForEntry (ICLEntry *entry)
	{
		IAccount *account =
				qobject_cast<IAccount*> (entry->GetParentAccount ());
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< "parent account doesn't implement IAccount:"
					<< entry->GetParentAccount ();
			return;
		}
		account->DenyAuth (entry->GetObject ());

		const auto& e = Util::MakeANCancel ("org.LeechCraft.Azoth",
				"org.LC.Plugins.Azoth.AuthRequestFrom/" + entry->GetEntryID ());
		Core::Instance ().SendEntity (e);
	}
}
}
