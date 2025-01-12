/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "util.h"
#include <cmath>
#include <QString>
#include <QWizard>
#include <QList>
#include <QMessageBox>
#include <QMainWindow>
#include <util/threads/futures.h>
#include <util/xpc/util.h>
#include <interfaces/an/constants.h>
#include <interfaces/structures.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/iregmanagedaccount.h"

#ifdef ENABLE_CRYPT
#include "interfaces/azoth/isupportpgp.h"
#include "components/dialogs/pgpkeyselectiondialog.h"
#endif

#include "addaccountwizardfirstpage.h"
#include "core.h"
#include "chattabsmanager.h"
#include "xmlsettingsmanager.h"
#include "avatarsmanager.h"

Q_DECLARE_METATYPE (QList<QColor>);

namespace LC
{
namespace Azoth
{
	QWidget* GetDialogParent ()
	{
		return GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
	}

	QFuture<Entity> BuildNotification (AvatarsManager *avatarsMgr,
			Entity e, ICLEntry *other, const QString& id, ICLEntry *avatarSource)
	{
		e.Additional_ ["org.LC.AdvNotifications.SenderID"] = "org.LeechCraft.Azoth";
		e.Additional_ ["org.LC.AdvNotifications.EventCategory"] = AN::CatIM;
		e.Additional_ ["org.LC.AdvNotifications.EventID"] =
				"org.LC.Plugins.Azoth." + (id.isEmpty () ? "IncomingMessageFrom/" : id) + other->GetEntryID ();

		e.Additional_ ["org.LC.AdvNotifications.VisualPath"] = QStringList (other->GetEntryName ());

		int win = 0;
		const auto tab = Core::Instance ()
				.GetChatTabsManager ()->GetChatTab (other->GetEntryID ());

		const auto rootWM = Core::Instance ().GetProxy ()->GetRootWindowsManager ();
		if (tab)
			win = rootWM->GetWindowForTab (tab);
		if (!tab || win == -1)
		{
			const auto& tc = other->GetEntryType () == ICLEntry::EntryType::MUC ?
					ChatTab::GetMUCTabClassInfo () :
					ChatTab::GetChatTabClassInfo ();
			win = rootWM->GetPreferredWindowIndex (tc.TabClass_);
		}

		e.Additional_ ["org.LC.AdvNotifications.WindowIndex"] = win;

		e.Additional_ ["org.LC.Plugins.Azoth.SourceName"] = other->GetEntryName ();
		e.Additional_ ["org.LC.Plugins.Azoth.SourceID"] = other->GetEntryID ();
		e.Additional_ ["org.LC.Plugins.Azoth.SourceGroups"] = other->Groups ();

		if (const auto parent = other->GetParentCLEntry ())
		{
			e.Additional_ ["org.LC.Plugins.Azoth.ParentSourceID"] = parent->GetEntryID ();
			e.Additional_ ["org.LC.Plugins.Azoth.ParentSourceName"] = parent->GetEntryName ();
		}

		if (!avatarSource)
			avatarSource = other;

		QPointer<AvatarsManager> mgr { avatarsMgr };
		QPointer<QObject> src { avatarSource->GetQObject () };
		const auto& avatarFutureGetter = [mgr, src] () -> Util::LazyNotificationPixmap_t::result_type
		{
			if (mgr && src)
				return mgr->GetAvatar (src, IHaveAvatars::Size::Thumbnail);
			else
				return {};
		};
		e.Additional_ ["NotificationPixmap"] = QVariant::fromValue<Util::LazyNotificationPixmap_t> (avatarFutureGetter);
		return Util::MakeReadyFuture (e);
	}

	QString GetActivityIconName (const QString& general, const QString& specific)
	{
		return (general + ' ' + specific).trimmed ().replace (' ', '_');
	}

	void InitiateAccountAddition (QWidget *parent)
	{
		QWizard *wizard = new QWizard (parent);
		wizard->setAttribute (Qt::WA_DeleteOnClose);
		wizard->setWindowTitle (QObject::tr ("Add account"));
		wizard->addPage (new AddAccountWizardFirstPage (wizard));

		wizard->show ();
	}

	void AuthorizeEntry (ICLEntry *entry)
	{
		const auto account = entry->GetParentAccount ();
		const auto& id = entry->GetHumanReadableID ();
		account->Authorize (entry->GetQObject ());
		account->RequestAuth (id);

		const auto& e = Util::MakeANCancel ("org.LeechCraft.Azoth",
				"org.LC.Plugins.Azoth.AuthRequestFrom/" + entry->GetEntryID ());
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	void DenyAuthForEntry (ICLEntry *entry)
	{
		const auto account = entry->GetParentAccount ();
		account->DenyAuth (entry->GetQObject ());

		const auto& e = Util::MakeANCancel ("org.LeechCraft.Azoth",
				"org.LC.Plugins.Azoth.AuthRequestFrom/" + entry->GetEntryID ());
		GetProxyHolder ()->GetEntityManager ()->HandleEntity (e);
	}

	QObject* FindByHRId (IAccount *acc, const QString& hrId)
	{
		const auto& allEntries = acc->GetCLEntries ();
		const auto pos = std::find_if (allEntries.begin (), allEntries.end (),
				[&hrId] (QObject *obj)
				{
					return qobject_cast<ICLEntry*> (obj)->GetHumanReadableID () == hrId;
				});

		return pos == allEntries.end () ? nullptr : *pos;
	}

	QList<QColor> GenerateColors (const QString& coloring, QColor bg)
	{
		QList<QColor> result;
		if (XmlSettingsManager::Instance ().property ("OverrideHashColors").toBool ())
		{
			result = XmlSettingsManager::Instance ().property ("OverrideColorsList").value<decltype (result)> ();
			if (!result.isEmpty ())
				return result;
		}

		auto compatibleColors = [] (const QColor& c1, const QColor& c2) -> bool
		{
			int dR = c1.red () - c2.red ();
			int dG = c1.green () - c2.green ();
			int dB = c1.blue () - c2.blue ();

			double dV = std::abs (c1.value () - c2.value ());
			double dC = std::sqrt (0.2126 * dR * dR + 0.7152 * dG * dG + 0.0722 * dB * dB);

			if ((dC < 80. && dV > 100.) ||
					(dC < 110. && dV <= 100. && dV > 10.) ||
					(dC < 125. && dV <= 10.))
				return false;

			return true;
		};

		if (coloring == "hash" || coloring.isEmpty ())
		{
			if (!bg.isValid ())
				bg = QApplication::palette ().color (QPalette::Base);

			int alpha = bg.alpha ();

			QColor color;
			for (int hue = 0; hue < 360; hue += 18)
			{
				color.setHsv (hue, 255, 255, alpha);
				if (compatibleColors (color, bg))
					result << color;
				color.setHsv (hue, 255, 170, alpha);
				if (compatibleColors (color, bg))
					result << color;
			}
		}
		else
			for (const auto& str : QStringView { coloring }.split (' ', Qt::SkipEmptyParts))
				result << QColor { str };

		return result;
	}

	QString GetNickColor (const QString& nick, const QList<QColor>& colors)
	{
		if (colors.isEmpty ())
			return "green";

		int hash = 0;
		for (int i = 0; i < nick.length (); ++i)
		{
			const QChar& c = nick.at (i);
			hash += c.toLatin1 () ?
					c.toLatin1 () :
					c.unicode ();
			hash += nick.length ();
		}
		hash = std::abs (hash);
		const auto& nc = colors.at (hash % colors.size ());
		return nc.name ();
	}

	QStringList GetMucParticipants (const QString& entryId)
	{
		const auto entry = qobject_cast<IMUCEntry*> (Core::Instance ().GetEntry (entryId));
		if (!entry)
		{
			qWarning () << Q_FUNC_INFO
					<< entry
					<< "doesn't implement IMUCEntry";
			return {};
		}

		QStringList participantsList;
		for (const auto item : entry->GetParticipants ())
		{
			const auto part = qobject_cast<ICLEntry*> (item);
			if (!part)
			{
				qWarning () << Q_FUNC_INFO
						<< "unable to cast item to ICLEntry"
						<< item;
				continue;
			}
			participantsList << part->GetEntryName ();
		}
		return participantsList;
	}

	void RemoveAccount (IAccount *acc)
	{
		if (!acc)
			return;

		if (QMessageBox::question (nullptr,
					"LeechCraft",
					QObject::tr ("Are you sure you want to remove the account %1?")
						.arg (acc->GetAccountName ()),
					QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
			return;

		if (const auto irm = qobject_cast<IRegManagedAccount*> (acc->GetQObject ()))
			if (irm->SupportsFeature (IRegManagedAccount::Feature::DeregisterAcc) &&
					QMessageBox::question (nullptr,
							"LeechCraft",
							QObject::tr ("Do you also want to remove %1 from the server?")
								.arg (acc->GetAccountName ()),
							QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
			{
				irm->DeregisterAccount ();
				return;
			}

		auto protoObj = acc->GetParentProtocol ();
		auto proto = qobject_cast<IProtocol*> (protoObj);
		if (!proto)
		{
			qWarning () << Q_FUNC_INFO
					<< "parent protocol for"
					<< acc->GetAccountID ()
					<< "doesn't implement IProtocol";
			return;
		}
		proto->RemoveAccount (acc->GetQObject ());
	}

	QString StateToString (State st)
	{
		switch (st)
		{
		case SOnline:
			return Core::tr ("Online");
		case SChat:
			return Core::tr ("Free to chat");
		case SAway:
			return Core::tr ("Away");
		case SDND:
			return Core::tr ("Do not disturb");
		case SXA:
			return Core::tr ("Not available");
		case SOffline:
			return Core::tr ("Offline");
		default:
			return Core::tr ("Error");
		}
	}

	QString PrettyPrintDateTime (const QDateTime& dt)
	{
		static const auto format = []
		{
			return QLocale {}.dateTimeFormat ()
					.remove (" t")
					.remove ("t ")
					.remove ("t");
		} ();
		return QLocale {}.toString (dt, format);
	}

	bool ChoosePGPKey (ISupportPGP *pgp, ICLEntry *entry)
	{
#ifdef ENABLE_CRYPT
		const auto& str = QObject::tr ("Please select the key for %1 (%2).")
				.arg (entry->GetEntryName ())
				.arg (entry->GetHumanReadableID ());
		PGPKeySelectionDialog dia { str, PGPKeySelectionDialog::TPublic,
				pgp->GetEntryKey (entry->GetQObject ()) };
		if (dia.exec () != QDialog::Accepted)
			return false;

		const auto& key = dia.GetSelectedKey ();

		pgp->SetEntryKey (entry->GetQObject (), key);

		QSettings settings { QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth" };
		settings.beginGroup ("PublicEntryKeys");
		if (key.isNull ())
			settings.remove (entry->GetEntryID ());
		else
			settings.setValue (entry->GetEntryID (), key.keyId ());
		settings.endGroup ();

		return !key.isNull ();
#else
		return false;
#endif
	}
}
}
