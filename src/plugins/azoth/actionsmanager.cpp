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

#include "actionsmanager.h"
#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QClipboard>
#include <util/util.h>
#include <util/defaulthookproxy.h>
#include <interfaces/core/icoreproxy.h>
#include "interfaces/iclentry.h"
#include "interfaces/imucperms.h"
#include "interfaces/iadvancedclentry.h"
#include "interfaces/imucentry.h"
#include "interfaces/iaccount.h"

#ifdef ENABLE_CRYPT
#include "interfaces/isupportpgp.h"
#include "pgpkeyselectiondialog.h"
#endif

#include "core.h"
#include "util.h"
#include "xmlsettingsmanager.h"
#include "chattabsmanager.h"
#include "drawattentiondialog.h"
#include "groupeditordialog.h"
#include "shareriexdialog.h"
#include "pgpkeyselectiondialog.h"
#include "mucinvitedialog.h"
#include "addcontactdialog.h"

namespace LeechCraft
{
namespace Azoth
{
	ActionsManager::ActionsManager (QObject *parent)
	: QObject (parent)
	{
	}

	QList<QAction*> ActionsManager::GetEntryActions (ICLEntry *entry)
	{
		if (!entry)
			return QList<QAction*> ();

		if (!Entry2Actions_.contains (entry))
			CreateActionsForEntry (entry);
		UpdateActionsForEntry (entry);

		const QHash<QByteArray, QAction*>& id2action = Entry2Actions_ [entry];
		QList<QAction*> result;
		result << id2action.value ("openchat");
		result << id2action.value ("drawattention");
		result << id2action.value ("sep_afterinitiate");
		result << id2action.value ("rename");
		result << id2action.value ("changegroups");
		result << id2action.value ("remove");
		result << id2action.value ("sep_afterrostermodify");
		result << id2action.value ("authorization");
		IMUCPerms *perms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntry ());
		if (perms)
			Q_FOREACH (const QByteArray& permClass, perms->GetPossiblePerms ().keys ())
				result << id2action.value (permClass);
		result << id2action.value ("sep_afterroles");
		result << id2action.value ("add_contact");
		result << id2action.value ("copy_id");
		result << id2action.value ("sep_afterjid");
		result << id2action.value ("managepgp");
		result << id2action.value ("shareRIEX");
		result << id2action.value ("vcard");
		result << id2action.value ("invite");
		result << id2action.value ("leave");
		result << id2action.value ("authorize");
		result << id2action.value ("denyauth");
		result << entry->GetActions ();

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		proxy->SetReturnValue (QVariantList ());
		emit hookEntryActionsRequested (proxy, entry->GetObject ());
		Q_FOREACH (const QVariant& var, proxy->GetReturnValue ().toList ())
		{
			QObject *obj = var.value<QObject*> ();
			QAction *act = qobject_cast<QAction*> (obj);
			if (!act)
				continue;

			result << act;

			proxy.reset (new Util::DefaultHookProxy);
			emit hookEntryActionAreasRequested (proxy, act, entry->GetObject ());
			Q_FOREACH (const QString& place, proxy->GetReturnValue ().toStringList ())
			{
				if (place == "contactListContextMenu")
					Action2Areas_ [act] << CLEAAContactListCtxtMenu;
				else if (place == "tabContextMenu")
					Action2Areas_ [act] << CLEAATabCtxtMenu;
				else if (place == "applicationMenu")
					Action2Areas_ [act] << CLEAAApplicationMenu;
				else if (place == "toolbar")
					Action2Areas_ [act] << CLEAAToolbar;
				else
					qWarning () << Q_FUNC_INFO
							<< "unknown embed place ID"
							<< place;
			}
		}

		result.removeAll (0);

		Core::Instance ().GetProxy ()->UpdateIconset (result);

		return result;
	}

	QList<ActionsManager::CLEntryActionArea> ActionsManager::GetAreasForAction (const QAction *action) const
	{
		return Action2Areas_.value (action,
				QList<CLEntryActionArea> () << CLEAAContactListCtxtMenu);
	}

	void ActionsManager::HandleEntryRemoved (ICLEntry *entry)
	{
		QHash<QByteArray, QAction*> actions = Entry2Actions_.take (entry);
		Q_FOREACH (QAction *action, actions.values ())
			Action2Areas_.remove (action);
	}

	void ActionsManager::CreateActionsForEntry (ICLEntry *entry)
	{
		if (!entry)
			return;

		IAdvancedCLEntry *advEntry = qobject_cast<IAdvancedCLEntry*> (entry->GetObject ());

		if (Entry2Actions_.contains (entry))
			Q_FOREACH (const QAction *action,
						Entry2Actions_.take (entry).values ())
			{
				Action2Areas_.remove (action);
				delete action;
			}

		QAction *openChat = new QAction (tr ("Open chat"), entry->GetObject ());
		openChat->setProperty ("ActionIcon", "azoth_openchat");
		connect (openChat,
				SIGNAL (triggered ()),
				this,
				SLOT (handleActionOpenChatTriggered ()));
		Entry2Actions_ [entry] ["openchat"] = openChat;
		Action2Areas_ [openChat] << CLEAAContactListCtxtMenu;

		if (advEntry)
		{
			QAction *drawAtt = new QAction (tr ("Draw attention..."), entry->GetObject ());
			connect (drawAtt,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionDrawAttention ()));
			drawAtt->setProperty ("ActionIcon", "draw_attention");
			Entry2Actions_ [entry] ["drawattention"] = drawAtt;
			Action2Areas_ [drawAtt] << CLEAAContactListCtxtMenu;
		}

		QAction *rename = new QAction (tr ("Rename"), entry->GetObject ());
		connect (rename,
				SIGNAL (triggered ()),
				this,
				SLOT (handleActionRenameTriggered ()));
		rename->setProperty ("ActionIcon", "azoth_rename");
		Entry2Actions_ [entry] ["rename"] = rename;
		Action2Areas_ [rename] << CLEAAContactListCtxtMenu;

		if (entry->GetEntryFeatures () & ICLEntry::FSupportsGrouping)
		{
			QAction *changeGroups = new QAction (tr ("Change groups..."), entry->GetObject ());
			connect (changeGroups,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionChangeGroupsTriggered ()));
			changeGroups->setProperty ("ActionIcon", "azoth_changegroups");
			Entry2Actions_ [entry] ["changegroups"] = changeGroups;
			Action2Areas_ [changeGroups] << CLEAAContactListCtxtMenu;
		}

		if (entry->GetEntryFeatures () & ICLEntry::FSupportsAuth)
		{
			QMenu *authMenu = new QMenu (tr ("Authorization"));
			authMenu->menuAction ()->setProperty ("ActionIcon", "azoth_menu_authorization");
			Entry2Actions_ [entry] ["authorization"] = authMenu->menuAction ();
			Action2Areas_ [authMenu->menuAction ()] << CLEAAContactListCtxtMenu;

			QAction *grantAuth = authMenu->addAction (tr ("Grant"),
					this, SLOT (handleActionGrantAuthTriggered ()));
			grantAuth->setProperty ("ActionIcon", "azoth_auth_grant");
			grantAuth->setProperty ("Azoth/WithReason", false);

			QAction *grantAuthReason = authMenu->addAction (tr ("Grant with reason..."),
					this, SLOT (handleActionGrantAuthTriggered ()));
			grantAuthReason->setProperty ("ActionIcon", "azoth_auth_grant");
			grantAuthReason->setProperty ("Azoth/WithReason", true);

			QAction *revokeAuth = authMenu->addAction (tr ("Revoke"),
					this, SLOT (handleActionRevokeAuthTriggered ()));
			revokeAuth->setProperty ("ActionIcon", "azoth_auth_revoke");
			revokeAuth->setProperty ("Azoth/WithReason", false);

			QAction *revokeAuthReason = authMenu->addAction (tr ("Revoke with reason..."),
					this, SLOT (handleActionRevokeAuthTriggered ()));
			revokeAuthReason->setProperty ("ActionIcon", "azoth_auth_revoke_reason");
			revokeAuthReason->setProperty ("Azoth/WithReason", true);

			QAction *unsubscribe = authMenu->addAction (tr ("Unsubscribe"),
					this, SLOT (handleActionUnsubscribeTriggered ()));
			unsubscribe->setProperty ("ActionIcon", "azoth_auth_unsubscribe");
			unsubscribe->setProperty ("Azoth/WithReason", false);

			QAction *unsubscribeReason = authMenu->addAction (tr ("Unsubscribe with reason..."),
					this, SLOT (handleActionUnsubscribeTriggered ()));
			unsubscribeReason->setProperty ("ActionIcon", "azoth_auth_unsubscribe");
			unsubscribeReason->setProperty ("Azoth/WithReason", true);

			QAction *rerequest = authMenu->addAction (tr ("Rerequest authentication"),
					this, SLOT (handleActionRerequestTriggered ()));
			rerequest->setProperty ("ActionIcon", "azoth_auth_rerequest");
			rerequest->setProperty ("Azoth/WithReason", false);

			QAction *rerequestReason = authMenu->addAction (tr ("Rerequest authentication with reason..."),
					this, SLOT (handleActionRerequestTriggered ()));
			rerequestReason->setProperty ("ActionIcon", "azoth_auth_rerequest");
			rerequestReason->setProperty ("Azoth/WithReason", true);
		}

#ifdef ENABLE_CRYPT
		if (qobject_cast<ISupportPGP*> (entry->GetParentAccount ()))
		{
			QAction *manageGPG = new QAction (tr ("Manage PGP keys..."), entry->GetObject ());
			connect (manageGPG,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionManagePGPTriggered ()));
			manageGPG->setProperty ("ActionIcon", "encryption");
			Entry2Actions_ [entry] ["managepgp"] = manageGPG;
			Action2Areas_ [manageGPG] << CLEAAContactListCtxtMenu;
		}
#endif

		if (qobject_cast<ISupportRIEX*> (entry->GetParentAccount ()))
		{
			QAction *shareRIEX = new QAction (tr ("Share contacts..."), entry->GetObject ());
			connect (shareRIEX,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionShareContactsTriggered ()));
			Entry2Actions_ [entry] ["shareRIEX"] = shareRIEX;
			Action2Areas_ [shareRIEX] << CLEAAContactListCtxtMenu;
		}

		if (entry->GetEntryType () != ICLEntry::ETMUC)
		{
			QAction *vcard = new QAction (tr ("VCard"), entry->GetObject ());
			connect (vcard,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionVCardTriggered ()));
			vcard->setProperty ("ActionIcon", "personalinfo");
			Entry2Actions_ [entry] ["vcard"] = vcard;
			Action2Areas_ [vcard] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu
					<< CLEAAToolbar;
		}

		IMUCPerms *perms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntry ());
		if (entry->GetEntryType () == ICLEntry::ETPrivateChat)
		{
			if (perms)
			{
				const QMap<QByteArray, QList<QByteArray> >& possible = perms->GetPossiblePerms ();
				Q_FOREACH (const QByteArray& permClass, possible.keys ())
				{
					QMenu *changeClass = new QMenu (perms->GetUserString (permClass));
					Entry2Actions_ [entry] [permClass] = changeClass->menuAction ();
					Action2Areas_ [changeClass->menuAction ()] << CLEAAContactListCtxtMenu;

					Q_FOREACH (const QByteArray& perm, possible [permClass])
					{
						QAction *permAct = changeClass->addAction (perms->GetUserString (perm),
								this,
								SLOT (handleActionPermTriggered ()));
						permAct->setParent (entry->GetObject ());
						permAct->setCheckable (true);
						permAct->setProperty ("Azoth/TargetPermClass", permClass);
						permAct->setProperty ("Azoth/TargetPerm", perm);
					}
				}

				QAction *sep = Util::CreateSeparator (entry->GetObject ());
				Entry2Actions_ [entry] ["sep_afterroles"] = sep;
				Action2Areas_ [sep] << CLEAAContactListCtxtMenu;
			}

			QAction *addContact = new QAction (tr ("Add to contact list..."), entry->GetObject ());
			addContact->setProperty ("ActionIcon", "add");
			connect (addContact,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionAddContactFromMUC ()));
			Entry2Actions_ [entry] ["add_contact"] = addContact;
			Action2Areas_ [addContact] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu;

			QAction *copyId = new QAction (tr ("Copy ID"), entry->GetObject ());
			copyId->setProperty ("ActionIcon", "copy");
			connect (copyId,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionCopyMUCPartID ()));
			Entry2Actions_ [entry] ["copy_id"] = copyId;
			Action2Areas_ [copyId] << CLEAAContactListCtxtMenu;

			QAction *sep = Util::CreateSeparator (entry->GetObject ());
			Entry2Actions_ [entry] ["sep_afterjid"] = sep;
			Action2Areas_ [sep] << CLEAAContactListCtxtMenu;
		}
		else if (entry->GetEntryType () == ICLEntry::ETMUC)
		{
			QAction *invite = new QAction (tr ("Invite..."), entry->GetObject ());
			invite->setProperty ("ActionIcon", "azoth_invite");
			connect (invite,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionInviteTriggered ()));
			Entry2Actions_ [entry] ["invite"] = invite;
			Action2Areas_ [invite] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu;

			QAction *leave = new QAction (tr ("Leave"), entry->GetObject ());
			leave->setProperty ("ActionIcon", "azoth_leave");
			connect (leave,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionLeaveTriggered ()));
			Entry2Actions_ [entry] ["leave"] = leave;
			Action2Areas_ [leave] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu;
		}
		else if (entry->GetEntryType () == ICLEntry::ETUnauthEntry)
		{
			QAction *authorize = new QAction (tr ("Authorize"), entry->GetObject ());
			authorize->setProperty ("ActionIcon", "azoth_authorize");
			connect (authorize,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionAuthorizeTriggered ()));
			Entry2Actions_ [entry] ["authorize"] = authorize;
			Action2Areas_ [authorize] << CLEAAContactListCtxtMenu;

			QAction *denyAuth = new QAction (tr ("Deny authorization"), entry->GetObject ());
			denyAuth->setProperty ("ActionIcon", "azoth_denyauth");
			connect (denyAuth,
						SIGNAL (triggered ()),
						this,
						SLOT (handleActionDenyAuthTriggered ()));
			Entry2Actions_ [entry] ["denyauth"] = denyAuth;
			Action2Areas_ [denyAuth] << CLEAAContactListCtxtMenu;
		}
		else if (entry->GetEntryType () == ICLEntry::ETChat)
		{
			QAction *remove = new QAction (tr ("Remove"), entry->GetObject ());
			remove->setProperty ("ActionIcon", "remove");
			connect (remove,
					SIGNAL (triggered ()),
					this,
					SLOT (handleActionRemoveTriggered ()));
			Entry2Actions_ [entry] ["remove"] = remove;
			Action2Areas_ [remove] << CLEAAContactListCtxtMenu;
		}

		QAction *sep = Util::CreateSeparator (entry->GetObject ());
		Entry2Actions_ [entry] ["sep_afterinitiate"] = sep;
		Action2Areas_ [sep] << CLEAAContactListCtxtMenu;
		sep = Util::CreateSeparator (entry->GetObject ());
		Entry2Actions_ [entry] ["sep_afterrostermodify"] = sep;
		Action2Areas_ [sep] << CLEAAContactListCtxtMenu;

		struct Entrifier
		{
			QVariant Entry_;

			Entrifier (const QVariant& entry)
			: Entry_ (entry)
			{
			}

			void Do (const QList<QAction*>& actions)
			{
				Q_FOREACH (QAction *act, actions)
				{
					act->setProperty ("Azoth/Entry", Entry_);
					act->setParent (Entry_.value<ICLEntry*> ()->GetObject ());
					QMenu *menu = act->menu ();
					if (menu)
						Do (menu->actions ());
				}
			}
		} entrifier (QVariant::fromValue<ICLEntry*> (entry));
		entrifier.Do (Entry2Actions_ [entry].values ());
	}

	void ActionsManager::UpdateActionsForEntry (ICLEntry *entry)
	{
		if (!entry)
			return;

		IAdvancedCLEntry *advEntry = qobject_cast<IAdvancedCLEntry*> (entry->GetObject ());

		IAccount *account = qobject_cast<IAccount*> (entry->GetParentAccount ());
		const bool isOnline = account->GetState ().State_ != SOffline;
		if (entry->GetEntryType () != ICLEntry::ETMUC)
		{
			bool enableVCard =
					account->GetAccountFeatures () & IAccount::FCanViewContactsInfoInOffline ||
					isOnline;
			Entry2Actions_ [entry] ["vcard"]->setEnabled (enableVCard);
		}

		Entry2Actions_ [entry] ["rename"]->setEnabled (entry->GetEntryFeatures () & ICLEntry::FSupportsRenames);

		if (advEntry)
		{
			bool suppAtt = advEntry->GetAdvancedFeatures () & IAdvancedCLEntry::AFSupportsAttention;
			Entry2Actions_ [entry] ["drawattention"]->setEnabled (suppAtt);
		}

		if (entry->GetEntryType () == ICLEntry::ETChat)
		{
			Entry2Actions_ [entry] ["remove"]->setEnabled (isOnline);
			if (Entry2Actions_ [entry] ["authorization"])
				Entry2Actions_ [entry] ["authorization"]->setEnabled (isOnline);
		}

		IMUCEntry *thisMuc = qobject_cast<IMUCEntry*> (entry->GetObject ());
		if (thisMuc)
			Entry2Actions_ [entry] ["invite"]->
					setEnabled (thisMuc->GetMUCFeatures () & IMUCEntry::MUCFCanInvite);

		IMUCEntry *mucEntry =
				qobject_cast<IMUCEntry*> (entry->GetParentCLEntry ());
		if (entry->GetEntryType () == ICLEntry::ETPrivateChat &&
				!mucEntry)
			qWarning () << Q_FUNC_INFO
					<< "parent of"
					<< entry->GetObject ()
					<< entry->GetParentCLEntry ()
					<< "doesn't implement IMUCEntry";

		IMUCPerms *mucPerms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntry ());
		if (entry->GetEntryType () == ICLEntry::ETPrivateChat)
		{
			if (mucPerms)
			{
				const QMap<QByteArray, QList<QByteArray> > possible = mucPerms->GetPossiblePerms ();
				QObject *entryObj = entry->GetObject ();
				Q_FOREACH (const QByteArray& permClass, possible.keys ())
					Q_FOREACH (QAction *action,
							Entry2Actions_ [entry] [permClass]->menu ()->actions ())
					{
						const QByteArray& perm = action->property ("Azoth/TargetPerm").toByteArray ();
						action->setEnabled (mucPerms->MayChangePerm (entryObj,
									permClass, perm));
						action->setChecked (perm == mucPerms->GetPerms (entryObj) [permClass]);
					}
			}

			const QString& realJid = mucEntry->GetRealID (entry->GetObject ());
			Entry2Actions_ [entry] ["add_contact"]->setEnabled (!realJid.isEmpty ());
			Entry2Actions_ [entry] ["add_contact"]->setProperty ("Azoth/RealID", realJid);
			Entry2Actions_ [entry] ["copy_id"]->setEnabled (!realJid.isEmpty ());
			Entry2Actions_ [entry] ["copy_id"]->setProperty ("Azoth/RealID", realJid);
		}
	}

	void ActionsManager::handleActionOpenChatTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		Core::Instance ().GetChatTabsManager ()->OpenChat (entry);
	}

	void ActionsManager::handleActionDrawAttention ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IAdvancedCLEntry *advEntry = qobject_cast<IAdvancedCLEntry*> (entry->GetObject ());
		if (!advEntry)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetObject ()
					<< "doesn't implement IAdvancedCLEntry";
			return;
		}

		const QStringList& vars = entry->Variants ();
		DrawAttentionDialog dia (vars);
		if (dia.exec () != QDialog::Accepted)
			return;

		const QString& variant = dia.GetResource ();
		const QString& text = dia.GetText ();

		QStringList varsToDraw;
		if (!variant.isEmpty ())
			varsToDraw << variant;
		else if (vars.isEmpty ())
			varsToDraw << QString ();
		else
			varsToDraw = vars;

		Q_FOREACH (const QString& var, varsToDraw)
			advEntry->DrawAttention (text, var);
	}

	void ActionsManager::handleActionRenameTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();

		const QString& oldName = entry->GetEntryName ();
		const QString& newName = QInputDialog::getText (0,
				tr ("Rename contact"),
				tr ("Please enter new name for the contact %1:")
					.arg (oldName),
				QLineEdit::Normal,
				oldName);

		if (newName.isEmpty () ||
				oldName == newName)
			return;

		entry->SetEntryName (newName);
	}

	void ActionsManager::handleActionChangeGroupsTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();

		const QStringList& groups = entry->Groups ();
		const QStringList& allGroups = Core::Instance ().GetChatGroups ();

		GroupEditorDialog *dia = new GroupEditorDialog (groups, allGroups);
		if (dia->exec () != QDialog::Accepted)
			return;

		entry->SetGroups (dia->GetGroups ());
	}

	void ActionsManager::handleActionRemoveTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IAccount *account =
				qobject_cast<IAccount*> (entry->GetParentAccount ());
		if (!account)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetObject ()
					<< "doesn't return proper IAccount:"
					<< entry->GetParentAccount ();
			return;
		}

		account->RemoveEntry (entry->GetObject ());
	}

	void ActionsManager::handleActionGrantAuthTriggered()
	{
		Core::Instance ().ManipulateAuth ("grantauth",
				tr ("Enter reason for granting authorization to %1:"),
				&IAuthable::ResendAuth);
	}

	void ActionsManager::handleActionRevokeAuthTriggered ()
	{
		Core::Instance ().ManipulateAuth ("revokeauth",
				tr ("Enter reason for revoking authorization from %1:"),
				&IAuthable::RevokeAuth);
	}

	void ActionsManager::handleActionUnsubscribeTriggered ()
	{
		Core::Instance ().ManipulateAuth ("unsubscribe",
				tr ("Enter reason for unsubscribing from %1:"),
				&IAuthable::Unsubscribe);
	}

	void ActionsManager::handleActionRerequestTriggered ()
	{
		Core::Instance ().ManipulateAuth ("rerequestauth",
				tr ("Enter reason for rerequesting authorization from %1:"),
				&IAuthable::RerequestAuth);
	}


#ifdef ENABLE_CRYPT
	void ActionsManager::handleActionManagePGPTriggered ()
	{
		ICLEntry *entry = sender ()->
				property ("Azoth/Entry").value<ICLEntry*> ();

		QObject *accObj = entry->GetParentAccount ();
		IAccount *acc = qobject_cast<IAccount*> (accObj);
		ISupportPGP *pgp = qobject_cast<ISupportPGP*> (accObj);

		if (!pgp)
		{
			qWarning () << Q_FUNC_INFO
					<< accObj
					<< "doesn't implement ISupportPGP";
			QMessageBox::warning (0,
					"LeechCraft",
					tr ("The parent account %1 for entry %2 doesn't "
						"support encryption.")
							.arg (acc->GetAccountName ())
							.arg (entry->GetEntryName ()));
			return;
		}

		const QString& str = tr ("Please select the key for %1 (%2).")
				.arg (entry->GetEntryName ())
				.arg (entry->GetHumanReadableID ());
		PGPKeySelectionDialog dia (str, PGPKeySelectionDialog::TPublic);
		if (dia.exec () != QDialog::Accepted)
			return;

		const QCA::PGPKey& key = dia.GetSelectedKey ();

		pgp->SetEntryKey (entry->GetObject (), key);

		QSettings settings (QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth");
		settings.beginGroup ("PublicEntryKeys");
		if (key.isNull ())
			settings.remove (entry->GetEntryID ());
		else
			settings.setValue (entry->GetEntryID (), key.keyId ());
		settings.endGroup ();
	}
#endif

	void ActionsManager::handleActionShareContactsTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();

		ISupportRIEX *riex = qobject_cast<ISupportRIEX*> (entry->GetParentAccount ());
		if (!riex)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetParentAccount ()
					<< "doesn't implement ISupportRIEX";
			return;
		}

		ShareRIEXDialog dia (entry);
		if (dia.exec () != QDialog::Accepted)
			return;

		const bool shareGroups = dia.ShouldSuggestGroups ();

		QList<RIEXItem> items;
		Q_FOREACH (ICLEntry *toShare, dia.GetSelectedEntries ())
		{
			RIEXItem item =
			{
				RIEXItem::AAdd,
				toShare->GetHumanReadableID (),
				toShare->GetEntryName (),
				shareGroups ? toShare->Groups () : QStringList ()
			};
			items << item;
		}

		riex->SuggestItems (items, entry->GetObject (), dia.GetMessage ());
	}

	void ActionsManager::handleActionVCardTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		entry->ShowInfo ();
	}


	void ActionsManager::handleActionInviteTriggered ()
	{
		ICLEntry *entry = sender ()->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IMUCEntry *mucEntry =
				qobject_cast<IMUCEntry*> (entry->GetObject ());

		MUCInviteDialog dia (qobject_cast<IAccount*> (entry->GetParentAccount ()));
		if (dia.exec () != QDialog::Accepted)
			return;

		const QString& id = dia.GetID ();
		const QString& msg = dia.GetMessage ();
		if (id.isEmpty ())
			return;

		mucEntry->InviteToMUC (id, msg);
	}

	void ActionsManager::handleActionLeaveTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IMUCEntry *mucEntry =
				qobject_cast<IMUCEntry*> (entry->GetObject ());
		if (!mucEntry)
		{
			qWarning () << Q_FUNC_INFO
					<< "hm, requested leave on an entry"
					<< entry->GetObject ()
					<< "that doesn't implement IMUCEntry"
					<< sender ();
			return;
		}

		if (XmlSettingsManager::Instance ().property ("CloseConfOnLeave").toBool ())
		{
			Core::Instance ().GetChatTabsManager ()->CloseChat (entry);
			Q_FOREACH (QObject *partObj, mucEntry->GetParticipants ())
			{
				ICLEntry *partEntry = qobject_cast<ICLEntry*> (partObj);
				if (!partEntry)
				{
					qWarning () << Q_FUNC_INFO
							<< "unable to cast"
							<< partObj
							<< "to ICLEntry";
					continue;
				}

				Core::Instance ().GetChatTabsManager ()->CloseChat (partEntry);
			}
		}

		mucEntry->Leave ();
	}

	void ActionsManager::handleActionAuthorizeTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		AuthorizeEntry (entry);
	}

	void ActionsManager::handleActionDenyAuthTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		DenyAuthForEntry (entry);
	}

	void ActionsManager::handleActionAddContactFromMUC ()
	{
		const QString& id = sender ()->property ("Azoth/RealID").toString ();
		if (id.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty ID"
					<< sender ()
					<< sender ()->property ("Azoth/RealID");
			return;
		}

		ICLEntry *entry = sender ()->
				property ("Azoth/Entry").value<ICLEntry*> ();
		const QString& nick = entry->GetEntryName ();

		IAccount *account = qobject_cast<IAccount*> (entry->GetParentAccount ());

		std::auto_ptr<AddContactDialog> dia (new AddContactDialog (account));
		dia->SetContactID (id);
		dia->SetNick (nick);
		if (dia->exec () != QDialog::Accepted)
			return;

		dia->GetSelectedAccount ()->RequestAuth (dia->GetContactID (),
					dia->GetReason (),
					dia->GetNick (),
					dia->GetGroups ());
	}

	void ActionsManager::handleActionCopyMUCPartID ()
	{
		const QString& id = sender ()->property ("Azoth/RealID").toString ();
		if (id.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "empty ID"
					<< sender ()
					<< sender ()->property ("Azoth/RealID");
			return;
		}

		QApplication::clipboard ()->setText (id, QClipboard::Clipboard);
		QApplication::clipboard ()->setText (id, QClipboard::Selection);
	}

	void ActionsManager::handleActionPermTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		const QByteArray& permClass = action->property ("Azoth/TargetPermClass").toByteArray ();
		const QByteArray& perm = action->property ("Azoth/TargetPerm").toByteArray ();
		if (permClass.isEmpty () || perm.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "invalid perms set"
					<< action->property ("Azoth/TargetPermClass")
					<< action->property ("Azoth/TargetPerm");
			return;
		}

		ICLEntry *entry = action->
				property ("Azoth/Entry").value<ICLEntry*> ();
		IMUCPerms *mucPerms =
				qobject_cast<IMUCPerms*> (entry->GetParentCLEntry ());
		if (!mucPerms)
		{
			qWarning () << Q_FUNC_INFO
					<< entry->GetParentCLEntry ()
					<< "doesn't implement IMUCPerms";
			return;
		}

		mucPerms->SetPerm (entry->GetObject (), permClass, perm, QString ());
	}
}
}
