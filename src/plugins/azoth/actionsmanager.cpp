/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "actionsmanager.h"
#include <functional>
#include <algorithm>
#include <map>
#include <variant>
#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QClipboard>
#include <QFileDialog>
#include <QTimer>
#include <util/util.h>
#include <util/xpc/defaulthookproxy.h>
#include <util/shortcuts/shortcutmanager.h>
#include <util/sll/prelude.h>
#include <util/sll/visitor.h>
#include <util/sll/void.h>
#include <util/sys/util.h>
#include <util/threads/futures.h>
#include <util/xpc/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/an/constants.h>
#include "interfaces/azoth/iclentry.h"
#include "interfaces/azoth/imucperms.h"
#include "interfaces/azoth/iadvancedclentry.h"
#include "interfaces/azoth/imucentry.h"
#include "interfaces/azoth/iauthable.h"
#include "interfaces/azoth/iaccount.h"
#include "interfaces/azoth/itransfermanager.h"
#include "interfaces/azoth/iconfigurablemuc.h"
#include "interfaces/azoth/ihavedirectedstatus.h"
#include "interfaces/azoth/imucjoinwidget.h"
#include "interfaces/azoth/imucprotocol.h"
#include "interfaces/azoth/ihaveblacklists.h"
#include "interfaces/azoth/ihaveserverhistory.h"

#ifdef ENABLE_CRYPT
#include "interfaces/azoth/isupportpgp.h"
#endif

#include "core.h"
#include "util.h"
#include "xmlsettingsmanager.h"
#include "chattabsmanager.h"
#include "drawattentiondialog.h"
#include "groupeditordialog.h"
#include "shareriexdialog.h"
#include "mucinvitedialog.h"
#include "addcontactdialog.h"
#include "transferjobmanager.h"
#include "bookmarksmanagerdialog.h"
#include "simpledialog.h"
#include "setstatusdialog.h"
#include "filesenddialog.h"
#include "advancedpermchangedialog.h"
#include "proxyobject.h"
#include "serverhistorywidget.h"
#include "avatarsmanager.h"

using SingleEntryActor_f = std::function<void (LC::Azoth::ICLEntry*)> ;
using MultiEntryActor_f = std::function<void (QList<LC::Azoth::ICLEntry*>)> ;

using EntryActor_f = std::variant<LC::Util::Void, SingleEntryActor_f, MultiEntryActor_f>;
Q_DECLARE_METATYPE (EntryActor_f);

using EntriesList_t = QList<LC::Azoth::ICLEntry*>;
Q_DECLARE_METATYPE (EntriesList_t);

namespace LC
{
namespace Azoth
{
	namespace
	{
		void DrawAttention (ICLEntry *entry)
		{
			IAdvancedCLEntry *advEntry = qobject_cast<IAdvancedCLEntry*> (entry->GetQObject ());
			if (!advEntry)
			{
				qWarning () << Q_FUNC_INFO
						<< entry->GetQObject ()
						<< "doesn't implement IAdvancedCLEntry";
				return;
			}

			const auto& vars = entry->Variants ();

			DrawAttentionDialog dia (vars);
			if (dia.exec () != QDialog::Accepted)
				return;

			const auto& variant = dia.GetResource ();
			const auto& text = dia.GetText ();

			QStringList varsToDraw;
			if (!variant.isEmpty ())
				varsToDraw << variant;
			else if (vars.isEmpty ())
				varsToDraw << QString ();
			else
				varsToDraw = vars;

			for (const auto& var : varsToDraw)
				advEntry->DrawAttention (text, var);
		}

		void Rename (ICLEntry *entry)
		{
			const QString& oldName = entry->GetEntryName ();
			const QString& newName = QInputDialog::getText (0,
					ActionsManager::tr ("Rename contact"),
					ActionsManager::tr ("Please enter new name for the contact %1:")
						.arg (oldName),
					QLineEdit::Normal,
					oldName);

			if (newName.isEmpty () ||
					oldName == newName)
				return;

			entry->SetEntryName (newName);
		}

		void ChangeGroups (QList<ICLEntry*> entries)
		{
			const auto& groups = entries.first ()->Groups ();
			const auto& allGroups = Core::Instance ().GetChatGroups ();

			GroupEditorDialog dia (groups, allGroups);
			if (dia.exec () != QDialog::Accepted)
				return;

			const auto& newGroups = dia.GetGroups ();
			for (auto entry : entries)
				entry->SetGroups (newGroups);
		}

		void Remove (ICLEntry *entry)
		{
			entry->GetParentAccount ()->RemoveEntry (entry->GetQObject ());
		}

		QString GetMUCRealID (ICLEntry *entry)
		{
			const auto parentObj = entry->GetParentCLEntryObject ();
			const auto mucEntry = qobject_cast<IMUCEntry*> (parentObj);
			return mucEntry ?
					mucEntry->GetRealID (entry->GetQObject ()) :
					QString ();
		}

		void SendDirectedStatus (QList<ICLEntry*> entries)
		{
			QString variant;
			if (entries.size () == 1)
			{
				const auto entry = entries.front ();
				auto ihds = qobject_cast<IHaveDirectedStatus*> (entry->GetQObject ());

				QStringList variants (ActionsManager::tr ("All variants"));
				for (const QString& var : entry->Variants ())
					if (!var.isEmpty () &&
							ihds->CanSendDirectedStatusNow (var))
						variants << var;

				if (variants.size () > 2)
				{
					variant = QInputDialog::getItem (0,
							ActionsManager::tr ("Select variant"),
							ActionsManager::tr ("Select variant to send directed status to:"),
							variants,
							0,
							false);
					if (variant.isEmpty ())
						return;

					if (variant == variants.front ())
						variant.clear ();
				}
			}

			SetStatusDialog dia ((QString ()));
			if (dia.exec () != QDialog::Accepted)
				return;

			const EntryStatus st (dia.GetState (), dia.GetStatusText ());
			for (const auto entry : entries)
			{
				auto ihds = qobject_cast<IHaveDirectedStatus*> (entry->GetQObject ());
				ihds->SendDirectedStatus (st, variant);
			}
		}

		void AddContactFromMUC (ICLEntry *entry)
		{
			const auto& nick = entry->GetEntryName ();

			AddContactDialog dia { entry->GetParentAccount () };
			dia.SetContactID (GetMUCRealID (entry));
			dia.SetNick (nick);
			if (dia.exec () != QDialog::Accepted)
				return;

			dia.GetSelectedAccount ()->RequestAuth (dia.GetContactID (),
						dia.GetReason (),
						dia.GetNick (),
						dia.GetGroups ());
		}

		void CopyMUCParticipantID (ICLEntry *entry)
		{
			const auto& id = GetMUCRealID (entry);
			QApplication::clipboard ()->setText (id, QClipboard::Clipboard);
			QApplication::clipboard ()->setText (id, QClipboard::Selection);
		}

		void ViewServerHistory (ICLEntry *entry)
		{
			const auto accObj = entry->GetParentAccount ()->GetQObject ();
			const auto ihsh = qobject_cast<IHaveServerHistory*> (accObj);
			if (!ihsh || !ihsh->HasFeature (ServerHistoryFeature::AccountSupportsHistory))
				return;

			auto widget = new ServerHistoryWidget (ihsh);
			widget->SelectEntry (entry);
			GetProxyHolder ()->GetRootWindowsManager ()->AddTab (widget);
		}

#ifdef ENABLE_CRYPT
		void ManagePGP (ICLEntry *entry)
		{
			const auto acc = entry->GetParentAccount ();
			const auto accObj = acc->GetQObject ();
			const auto pgp = qobject_cast<ISupportPGP*> (accObj);

			if (!pgp)
			{
				qWarning () << Q_FUNC_INFO
						<< accObj
						<< "doesn't implement ISupportPGP";
				QMessageBox::warning (0,
						"LeechCraft",
						ActionsManager::tr ("The parent account %1 for entry %2 doesn't "
							"support encryption.")
								.arg (acc->GetAccountName ())
								.arg (entry->GetEntryName ()));
				return;
			}

			ChoosePGPKey (pgp, entry);
		}
#endif

		void ShareRIEX (ICLEntry *entry)
		{
			auto riex = qobject_cast<ISupportRIEX*> (entry->GetParentAccount ()->GetQObject ());
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

			const auto& items = Util::Map (dia.GetSelectedEntries (),
					[shareGroups] (ICLEntry *toShare) -> RIEXItem
					{
						return
						{
							RIEXItem::AAdd,
							toShare->GetHumanReadableID (),
							toShare->GetEntryName (),
							shareGroups ? toShare->Groups () : QStringList ()
						};
					});
			riex->SuggestItems (items, entry->GetQObject (), dia.GetShareMessage ());
		}

		void ChangeNick (const QList<ICLEntry*>& entries)
		{
			auto mucEntry = qobject_cast<IMUCEntry*> (entries.at (0)->GetQObject ());
			const auto& nick = mucEntry->GetNick ();

			const auto& newNick = QInputDialog::getText (nullptr,
					"LeechCraft",
					ActionsManager::tr ("Enter new nickname:"),
					QLineEdit::Normal,
					nick);
			if (newNick.isEmpty () || newNick == nick)
				return;

			for (auto entry : entries)
				qobject_cast<IMUCEntry*> (entry->GetQObject ())->SetNick (newNick);
		}

		void InviteToMuc (ICLEntry *entry)
		{
			QList<QObject*> mucObjs;

			const auto account = entry->GetParentAccount ();
			for (const auto entryObj : account->GetCLEntries ())
				if (qobject_cast<ICLEntry*> (entryObj)->GetEntryType () == ICLEntry::EntryType::MUC)
					mucObjs << entryObj;

			if (mucObjs.isEmpty ())
				return;

			MUCInviteDialog dia (account, MUCInviteDialog::ListType::ListMucs);
			if (dia.exec () != QDialog::Accepted)
				return;

			const auto mucEntryObj = FindByHRId (account, dia.GetID ());
			const auto mucEntry = qobject_cast<IMUCEntry*> (mucEntryObj);
			if (!mucEntry)
			{
				qWarning () << Q_FUNC_INFO
						<< "no MUC for"
						<< dia.GetID ();
				return;
			}

			const auto& msg = dia.GetInviteMessage ();
			mucEntry->InviteToMUC (entry->GetHumanReadableID (), msg);
		}

		void Invite (ICLEntry *entry)
		{
			auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());

			MUCInviteDialog dia { entry->GetParentAccount () };
			if (dia.exec () != QDialog::Accepted)
				return;

			const QString& id = dia.GetID ();
			const QString& msg = dia.GetInviteMessage ();
			if (id.isEmpty ())
				return;

			mucEntry->InviteToMUC (id, msg);
		}

		void Leave (ICLEntry *entry)
		{
			auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());
			if (!mucEntry)
			{
				qWarning () << Q_FUNC_INFO
						<< "hm, requested leave on an entry"
						<< entry->GetQObject ()
						<< "that doesn't implement IMUCEntry";
				return;
			}

			const bool closeTabs = XmlSettingsManager::Instance ().property ("CloseConfOnLeave").toBool ();
			if (closeTabs)
				for (auto partObj : mucEntry->GetParticipants ())
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

					Core::Instance ().GetChatTabsManager ()->CloseChat (partEntry, true);
				}

			mucEntry->Leave ();

			if (closeTabs)
				Core::Instance ().GetChatTabsManager ()->CloseChat (entry, true);
		}

		void Reconnect (ICLEntry *entry)
		{
			auto mucEntry = qobject_cast<IMUCEntry*> (entry->GetQObject ());
			if (!mucEntry)
			{
				qWarning () << Q_FUNC_INFO
						<< "requested reconnect on an entry"
						<< entry->GetQObject ()
						<< "that doesn't implement IMUCEntry";
				return;
			}

			const auto acc = entry->GetParentAccount ();
			const auto accObj = acc->GetQObject ();
			auto proto = qobject_cast<IMUCProtocol*> (acc->GetParentProtocol ());
			if (!proto)
			{
				qWarning () << Q_FUNC_INFO
						<< "requested reconnect on an entry"
						<< entry->GetHumanReadableID ()
						<< "whose parent account doesn't implement IMUCProtocol";
				return;
			}

			const auto& data = mucEntry->GetIdentifyingData ();
			mucEntry->Leave ();

			auto w = proto->GetMUCJoinWidget ();
			auto imjw = qobject_cast<IMUCJoinWidget*> (w);
			imjw->AccountSelected (accObj);
			imjw->SetIdentifyingData (data);

			QTimer::singleShot (1000,
					[w, imjw, accObj]
					{
						imjw->Join (accObj);
						w->deleteLater ();
					});
		}

		void ConfigureMUC (ICLEntry *entry)
		{
			QObject *entryObj = entry->GetQObject ();
			IConfigurableMUC *confMUC = qobject_cast<IConfigurableMUC*> (entryObj);
			if (!confMUC)
				return;

			QWidget *w = confMUC->GetConfigurationWidget ();
			if (!w)
			{
				qWarning () << Q_FUNC_INFO
						<< "empty conf widget"
						<< entryObj;
				return;
			}

			SimpleDialog *dia = new SimpleDialog ();
			dia->setWindowTitle (ActionsManager::tr ("Room configuration"));
			dia->SetWidget (w);
			QObject::connect (dia,
					SIGNAL (accepted ()),
					dia,
					SLOT (deleteLater ()),
					Qt::QueuedConnection);
			dia->show ();
		}

		void ChangePerm (QAction *action, ICLEntry* entry, const QString& text = QString (), bool global = false)
		{
			const auto& permClass = action->property ("Azoth/TargetPermClass").toByteArray ();
			const auto& perm = action->property ("Azoth/TargetPerm").toByteArray ();
			if (permClass.isEmpty () || perm.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "invalid perms set"
						<< action->property ("Azoth/TargetPermClass")
						<< action->property ("Azoth/TargetPerm");
				return;
			}

			const auto parentObj = entry->GetParentCLEntryObject ();
			auto muc = qobject_cast<IMUCEntry*> (parentObj);
			auto mucPerms = qobject_cast<IMUCPerms*> (parentObj);
			if (!muc || !mucPerms)
			{
				qWarning () << Q_FUNC_INFO
						<< parentObj
						<< "doesn't implement IMUCEntry or IMUCPerms";
				return;
			}

			const auto acc = entry->GetParentAccount ();
			const auto& realID = muc->GetRealID (entry->GetQObject ());

			mucPerms->SetPerm (entry->GetQObject (), permClass, perm, text);

			if (!global || realID.isEmpty ())
				return;

			for (auto item : acc->GetCLEntries ())
			{
				auto otherMuc = qobject_cast<IMUCEntry*> (item);
				if (!otherMuc || otherMuc == muc)
					continue;

				auto perms = qobject_cast<IMUCPerms*> (item);
				if (!perms)
					continue;

				bool found = false;
				for (auto part : otherMuc->GetParticipants ())
				{
					if (otherMuc->GetRealID (part) != realID)
						continue;

					found = true;

					if (perms->MayChangePerm (part, permClass, perm))
					{
						perms->SetPerm (part, permClass, perm, text);
						continue;
					}

					const auto& body = ActionsManager::tr ("Failed to change %1 for %2 in %3 "
							"due to insufficient permissions.")
							.arg (perms->GetUserString (permClass))
							.arg ("<em>" + realID + "</em>")
							.arg (qobject_cast<ICLEntry*> (item)->GetEntryName ());
					const auto& e = Util::MakeNotification ("Azoth", body, Priority::Warning);
					Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (e);
				}

				if (!found)
					perms->TrySetPerm (realID, permClass, perm, text);
			}
		}

		void ChangePermMulti (QAction *action, const QList<ICLEntry*>& entries, const QString& text = QString (), bool global = false)
		{
			for (const auto entry : entries)
				ChangePerm (action, entry, text, global);
		}

		void ChangePermAdvanced (QAction *action, const QList<ICLEntry*>& entries)
		{
			const auto& entry = entries.front ();

			if (!qobject_cast<IMUCPerms*> (entry->GetParentCLEntryObject ()))
				return;

			const auto& permClass = action->property ("Azoth/TargetPermClass").toByteArray ();
			const auto& perm = action->property ("Azoth/TargetPerm").toByteArray ();

			AdvancedPermChangeDialog dia (entries, permClass, perm);
			if (dia.exec () != QDialog::Accepted)
				return;

			const auto& text = dia.GetReason ();
			const auto isGlobal = dia.IsGlobal ();

			ChangePermMulti (action, entries, text, isGlobal);
		}
	}

	using ActionsVector_t = QList<QPair<QByteArray, EntryActor_f>>;

	struct ActionsManager::ActionsVectors
	{
		const ActionsVector_t BeforeRolesNames_;
		const ActionsVector_t AfterRolesNames_;

		ActionsVectors (AvatarsManager *am)
		: BeforeRolesNames_
		{
			{
				"openchat",
				SingleEntryActor_f ([] (ICLEntry *e)
						{ Core::Instance ().GetChatTabsManager ()->OpenChat (e, true); })
			},
			{ "drawattention", SingleEntryActor_f (DrawAttention) },
			{ "sendfile", SingleEntryActor_f ([] (ICLEntry *entry) { new FileSendDialog (entry); }) },
			{ "sep_afterinitiate", {} },
			{ "rename", SingleEntryActor_f (Rename) },
			{ "changegroups", MultiEntryActor_f (ChangeGroups) },
			{ "remove", SingleEntryActor_f (Remove) },
			{ "sep_afterrostermodify", {} },
			{ "directedpresence", MultiEntryActor_f (SendDirectedStatus) },
			{ "block", {} },
			{ "authorization", {} },
			{ "notifywhen", {} }
		}
		, AfterRolesNames_
		{
			{ "sep_afterroles", {} },
			{ "add_contact", SingleEntryActor_f (AddContactFromMUC) },
			{ "copy_muc_id", SingleEntryActor_f (CopyMUCParticipantID) },
			{ "sep_afterjid", {} },
			{ "view_server_history", SingleEntryActor_f (ViewServerHistory) },
#ifdef ENABLE_CRYPT
			{ "managepgp", SingleEntryActor_f (ManagePGP) },
#endif
			{ "shareRIEX", SingleEntryActor_f (ShareRIEX) },
			{
				"copy_id",
				SingleEntryActor_f ([] (ICLEntry *e) -> void
					{
						const auto& id = e->GetHumanReadableID ();
						QApplication::clipboard ()->setText (id, QClipboard::Clipboard);
					})
			},
			{ "inviteToMuc", SingleEntryActor_f (InviteToMuc) },
			{ "saveAvatar", SingleEntryActor_f ([am] (ICLEntry *e)
					{
						const auto entryObj = e->GetQObject ();
						Util::Sequence (entryObj, am->GetAvatar (entryObj, IHaveAvatars::Size::Full)) >>
								[] (const QImage& image)
								{
									if (image.isNull ())
										return;

									auto filename = QFileDialog::getSaveFileName (nullptr,
											ActionsManager::tr ("Save avatar"));
									if (filename.isEmpty ())
										return;

									const auto& supported = Util::HasSupportedImageExtension (filename);
									if (!supported)
										filename += ".png";

									image.save (filename);
								};
					}) },
			{ "vcard", SingleEntryActor_f ([] (ICLEntry *e) { e->ShowInfo (); }) },
			{ "sep_beforemuc", {} },
			{ "changenick", MultiEntryActor_f (ChangeNick) },
			{ "invite", SingleEntryActor_f (Invite) },
			{ "reconnect", SingleEntryActor_f (Reconnect) },
			{
				"addtobm",
				SingleEntryActor_f ([] (ICLEntry *e) -> void
					{
						auto dia = new BookmarksManagerDialog ();
						dia->SuggestSaving (e->GetQObject ());
						dia->show ();
					})
			},
			{ "configuremuc", SingleEntryActor_f (ConfigureMUC) },
			{
				"userslist",
				SingleEntryActor_f ([] (ICLEntry *e) -> void
					{
						auto chatWidget = Core::Instance ().GetChatTabsManager ()->OpenChat (e, false);
						auto tab = qobject_cast<ChatTab*> (chatWidget);
						tab->ShowUsersList ();
					})
			},
			{ "leave", SingleEntryActor_f (Leave) },
			{ "authorize", SingleEntryActor_f (AuthorizeEntry) },
			{ "denyauth", SingleEntryActor_f (DenyAuthForEntry) }
		}
		{
		}
	};

	ActionsManager::ActionsManager (AvatarsManager *am, QObject *parent)
	: QObject { parent }
	, AvatarsManager_ { am }
	, ActionsVectors_ { std::make_shared<ActionsVectors> (am) }
	{
	}

	QList<QAction*> ActionsManager::GetEntryActions (ICLEntry *entry)
	{
		if (!entry)
			return {};

		if (!Entry2Actions_.contains (entry))
			CreateActionsForEntry (entry);
		UpdateActionsForEntry (entry);

		const auto& id2action = Entry2Actions_ [entry];
		QList<QAction*> result;

		auto setter = [&result, &id2action, this] (const ActionsVector_t& pairs)
		{
			for (auto pair : pairs)
			{
				const auto& name = pair.first;
				const auto action = id2action.value (name);
				if (!action)
					continue;

				if (pair.second.index ())
					action->setProperty ("Azoth/EntryActor", QVariant::fromValue (pair.second));

				if (!action->property ("Azoth/EntryActor").isNull ())
					connect (action,
							SIGNAL (triggered ()),
							this,
							SLOT (handleActoredActionTriggered ()),
							Qt::UniqueConnection);

				result << action;
			}
		};

		setter (ActionsVectors_->BeforeRolesNames_);

		if (auto perms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntryObject ()))
			for (const auto& permClass : perms->GetPossiblePerms ().keys ())
				result << id2action.value (permClass);

		setter (ActionsVectors_->AfterRolesNames_);

		result << entry->GetActions ();

		Util::DefaultHookProxy_ptr proxy (new Util::DefaultHookProxy);
		proxy->SetReturnValue (QVariantList ());
		emit hookEntryActionsRequested (proxy, entry->GetQObject ());

		for (const auto& var : proxy->GetReturnValue ().toList ())
		{
			QObject *obj = var.value<QObject*> ();
			QAction *act = qobject_cast<QAction*> (obj);
			if (!act)
				continue;

			result << act;

			proxy.reset (new Util::DefaultHookProxy);
			emit hookEntryActionAreasRequested (proxy, act, entry->GetQObject ());
			for (const auto& place : proxy->GetReturnValue ().toStringList ())
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

		Core::Instance ().GetProxy ()->GetIconThemeManager ()->UpdateIconset (result);

		return result;
	}

	namespace
	{
		void DuplicateMenu (QAction *parentAction, QAction *refAction, QObject *receiver, const QList<ICLEntry*>& entries)
		{
			auto menu = new QMenu (parentAction->text ());
			parentAction->setMenu (menu);

			for (auto refSA : refAction->menu ()->actions ())
			{
				auto subAction = menu->addAction (refSA->text ());
				if (refSA->menu ())
				{
					DuplicateMenu (subAction, refSA, receiver, entries);
					continue;
				}

				subAction->setSeparator (refSA->isSeparator ());
				subAction->setProperty ("Azoth/Entries", QVariant::fromValue (entries));
				subAction->setProperty ("Azoth/EntryActor", refSA->property ("Azoth/EntryActor"));
				subAction->setProperty ("ActionIcon", refAction->property ("ActionIcon"));
				subAction->setProperty ("ReferenceAction", QVariant::fromValue<QObject*> (refAction));
				QObject::connect (subAction,
						SIGNAL (triggered ()),
						receiver,
						SLOT (handleActoredActionTriggered ()));
			}
		}
	}

	QList<QAction*> ActionsManager::CreateEntriesActions (QList<ICLEntry*> entries, QObject *parent)
	{
		entries.removeAll (nullptr);
		if (entries.isEmpty ())
			return {};

		for (auto entry : entries)
		{
			if (!Entry2Actions_.contains (entry))
				CreateActionsForEntry (entry);
			UpdateActionsForEntry (entry);
		}

		QList<QAction*> result;
		auto setter = [&result, &entries, parent, this] (const ActionsVector_t pairs) -> void
		{
			for (auto pair : pairs)
			{
				const auto& name = pair.first;

				if (!std::all_of (entries.begin (), entries.end (),
						[this, &name] (ICLEntry *entry) { return Entry2Actions_ [entry].value (name); }))
					continue;

				const auto refAction = Entry2Actions_ [entries.first ()] [name];
				const auto& refActorVar = refAction->property ("Azoth/EntryActor");
				if (refActorVar.isNull () &&
						!pair.second.index () &&
						!refAction->isSeparator () &&
						!refAction->menu ())
					continue;

				auto action = new QAction (refAction->text (), parent);
				if (!refAction->menu ())
				{
					action->setSeparator (refAction->isSeparator ());
					action->setProperty ("Azoth/Entries", QVariant::fromValue (entries));
					action->setProperty ("Azoth/EntryActor",
							refActorVar.isNull () ?
									QVariant::fromValue (pair.second) :
									refActorVar);
					action->setProperty ("ActionIcon", refAction->property ("ActionIcon"));
					action->setProperty ("ReferenceAction", QVariant::fromValue<QObject*> (refAction));
					connect (action,
							SIGNAL (triggered ()),
							this,
							SLOT (handleActoredActionTriggered ()));
				}
				else
					DuplicateMenu (action, refAction, this, entries);
				result << action;
			}
		};

		setter (ActionsVectors_->BeforeRolesNames_);

		if (const auto perms = qobject_cast<IMUCPerms*> (entries.front ()->GetParentCLEntryObject ()))
			if (std::all_of (entries.begin (), entries.end (), [perms] (ICLEntry *e)
					{ return perms == qobject_cast<IMUCPerms*> (e->GetParentCLEntryObject ()); }))
			{
				ActionsVector_t permPairs;
				const auto& id2action = Entry2Actions_ [entries.first ()];
				for (const auto& permClass : perms->GetPossiblePerms ().keys ())
				{
					const auto srcAct = id2action.value (permClass);
					const auto& actorVar = srcAct->property ("Azoth/EntryActor");
					permPairs.push_back ({ permClass, actorVar.value<EntryActor_f> () });
				}

				setter (permPairs);
			}

		setter (ActionsVectors_->AfterRolesNames_);

		Core::Instance ().GetProxy ()->GetIconThemeManager ()->UpdateIconset (result);

		return result;
	}

	QList<ActionsManager::CLEntryActionArea> ActionsManager::GetAreasForAction (const QAction *action) const
	{
		return Action2Areas_.value (action, { CLEAAContactListCtxtMenu });
	}

	void ActionsManager::HandleEntryRemoved (ICLEntry *entry)
	{
		auto actions = Entry2Actions_.take (entry);
		for (auto action : actions)
		{
			Action2Areas_.remove (action);
			delete action;
		}

		emit hookEntryActionsRemoved (std::make_shared<Util::DefaultHookProxy> (),
				entry->GetQObject ());
	}

	namespace
	{
		QString GetReason (const QString& text)
		{
			return QInputDialog::getText (0,
						ActionsManager::tr ("Enter reason"),
						text);
		}

		void ManipulateAuth (const QString& text, const QList<ICLEntry*>& entries, bool withReason,
				std::function<void (IAuthable*, const QString&)> func)
		{
			const auto& authables = Util::Map (entries, [] (ICLEntry *entry) { return qobject_cast<IAuthable*> (entry->GetQObject ()); });
			if (authables.isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "no authables in"
						<< entries;
				return;
			}

			QString reason;
			if (withReason)
			{
				const auto& arg = entries.size () == 1 ?
						entries.value (0)->GetEntryName () :
						ActionsManager::tr ("%n entry(ies)", 0, entries.size ());
				reason = GetReason (text.arg (arg));
				if (reason.isEmpty ())
					return;
			}

			for (const auto authable : authables)
				func (authable, reason);
		}
	}

	struct Entrifier
	{
		const QVariant Entry_;

		Entrifier (const QVariant& entry)
		: Entry_ { entry }
		{
		}

		template<typename T>
		void operator() (const T& actions)
		{
			for (const auto act : actions)
			{
				act->setProperty ("Azoth/Entry", Entry_);
				act->setParent (Entry_.value<ICLEntry*> ()->GetQObject ());
				if (const auto menu = act->menu ())
					(*this) (menu->actions ());
			}
		}
	};

	void ActionsManager::CreateActionsForEntry (ICLEntry *entry)
	{
		if (!entry)
			return;

		auto sm = Core::Instance ().GetShortcutManager ();

		const auto acc = entry->GetParentAccount ();

		IAdvancedCLEntry *advEntry = qobject_cast<IAdvancedCLEntry*> (entry->GetQObject ());

		if (Entry2Actions_.contains (entry))
			for (const auto action : Entry2Actions_.take (entry))
			{
				Action2Areas_.remove (action);
				delete action;
			}

		QAction *openChat = new QAction (tr ("Open chat"), entry->GetQObject ());
		openChat->setProperty ("ActionIcon", "view-conversation-balloon");
		Entry2Actions_ [entry] ["openchat"] = openChat;
		Action2Areas_ [openChat] << CLEAAContactListCtxtMenu;
		if (entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
			Action2Areas_ [openChat] << CLEAAChatCtxtMenu;

		auto copyEntryId = new QAction (tr ("Copy full entry ID"), entry->GetQObject ());
		copyEntryId->setProperty ("ActionIcon", "edit-copy");
		Action2Areas_ [copyEntryId] << CLEAAContactListCtxtMenu;
		Entry2Actions_ [entry] ["copy_id"] = copyEntryId;

		if (advEntry)
		{
			QAction *drawAtt = new QAction (tr ("Draw attention..."), entry->GetQObject ());
			drawAtt->setProperty ("ActionIcon", "emblem-important");
			Entry2Actions_ [entry] ["drawattention"] = drawAtt;
			Action2Areas_ [drawAtt] << CLEAAContactListCtxtMenu
					<< CLEAAToolbar;
		}

		if (qobject_cast<ITransferManager*> (acc->GetTransferManager ()))
		{
			QAction *sendFile = new QAction (tr ("Send file..."), entry->GetQObject ());
			sendFile->setProperty ("ActionIcon", "mail-attachment");
			Entry2Actions_ [entry] ["sendfile"] = sendFile;
			Action2Areas_ [sendFile] << CLEAAContactListCtxtMenu
					<< CLEAAToolbar;
		}

		QAction *rename = new QAction (tr ("Rename"), entry->GetQObject ());
		rename->setProperty ("ActionIcon", "edit-rename");
		Entry2Actions_ [entry] ["rename"] = rename;
		Action2Areas_ [rename] << CLEAAContactListCtxtMenu
				<< CLEAATabCtxtMenu;

		if (entry->GetEntryFeatures () & ICLEntry::FSupportsGrouping)
		{
			QAction *changeGroups = new QAction (tr ("Change groups..."), entry->GetQObject ());
			changeGroups->setProperty ("ActionIcon", "user-group-properties");
			Entry2Actions_ [entry] ["changegroups"] = changeGroups;
			Action2Areas_ [changeGroups] << CLEAAContactListCtxtMenu;
		}

		if (qobject_cast<IHaveDirectedStatus*> (entry->GetQObject ()))
		{
			QAction *sendDirected = new QAction (tr ("Send directed status..."), entry->GetQObject ());
			sendDirected->setProperty ("ActionIcon", "im-status-message-edit");
			Entry2Actions_ [entry] ["directedpresence"] = sendDirected;
			Action2Areas_ [sendDirected] << CLEAAContactListCtxtMenu;
		}

		if (entry->GetEntryFeatures () & ICLEntry::FSupportsAuth)
		{
			QMenu *authMenu = new QMenu (tr ("Authorization"));
			Entry2Actions_ [entry] ["authorization"] = authMenu->menuAction ();
			Action2Areas_ [authMenu->menuAction ()] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu;

			const auto grantAuth = authMenu->addAction (tr ("Grant"),
					this, SLOT (handleActoredActionTriggered ()));
			grantAuth->setProperty ("Azoth/EntryActor",
					QVariant::fromValue<EntryActor_f> ({
						[] (const QList<ICLEntry*>& entries)
						{
							ManipulateAuth (tr ("Enter reason for granting authorization to %1:"),
									entries,
									false,
									&IAuthable::ResendAuth);
						}
					}));

			const auto grantAuthReason = authMenu->addAction (tr ("Grant with reason..."),
					this, SLOT (handleActoredActionTriggered ()));
			grantAuthReason->setProperty ("Azoth/EntryActor",
					QVariant::fromValue<EntryActor_f> ({
						[] (const QList<ICLEntry*>& entries)
						{
							ManipulateAuth (tr ("Enter reason for granting authorization to %1:"),
									entries,
									true,
									&IAuthable::ResendAuth);
						}
					}));

			const auto revokeAuth = authMenu->addAction (tr ("Revoke"),
					this, SLOT (handleActoredActionTriggered ()));
			revokeAuth->setProperty ("Azoth/EntryActor",
					QVariant::fromValue<EntryActor_f> ({
						[] (const QList<ICLEntry*>& entries)
						{
							ManipulateAuth (tr ("Enter reason for revoking authorization from %1:"),
									entries,
									false,
									&IAuthable::RevokeAuth);
						}
					}));

			const auto revokeAuthReason = authMenu->addAction (tr ("Revoke with reason..."),
					this, SLOT (handleActoredActionTriggered ()));
			revokeAuthReason->setProperty ("Azoth/EntryActor",
					QVariant::fromValue<EntryActor_f> ({
						[] (const QList<ICLEntry*>& entries)
						{
							ManipulateAuth (tr ("Enter reason for revoking authorization from %1:"),
									entries,
									true,
									&IAuthable::RevokeAuth);
						}
					}));

			const auto unsubscribe = authMenu->addAction (tr ("Unsubscribe"),
					this, SLOT (handleActoredActionTriggered ()));
			unsubscribe->setProperty ("Azoth/EntryActor",
					QVariant::fromValue<EntryActor_f> ({
						[] (const QList<ICLEntry*>& entries)
						{
							ManipulateAuth (tr ("Enter reason for unsubscribing from %1:"),
									entries,
									false,
									&IAuthable::Unsubscribe);
						}
					}));

			const auto unsubscribeReason = authMenu->addAction (tr ("Unsubscribe with reason..."),
					this, SLOT (handleActoredActionTriggered ()));
			unsubscribeReason->setProperty ("Azoth/EntryActor",
					QVariant::fromValue<EntryActor_f> ({
						[] (const QList<ICLEntry*>& entries)
						{
							ManipulateAuth (tr ("Enter reason for unsubscribing from %1:"),
									entries,
									true,
									&IAuthable::Unsubscribe);
						}
					}));

			const auto rerequest = authMenu->addAction (tr ("Rerequest authentication"),
					this, SLOT (handleActoredActionTriggered ()));
			rerequest->setProperty ("Azoth/EntryActor",
					QVariant::fromValue<EntryActor_f> ({
						[] (const QList<ICLEntry*>& entries)
						{
							ManipulateAuth (tr ("Enter reason for rerequesting authorization from %1:"),
									entries,
									false,
									&IAuthable::RerequestAuth);
						}
					}));

			const auto rerequestReason = authMenu->addAction (tr ("Rerequest authentication with reason..."),
					this, SLOT (handleActoredActionTriggered ()));
			rerequestReason->setProperty ("Azoth/EntryActor",
					QVariant::fromValue<EntryActor_f> ({
						[] (const QList<ICLEntry*>& entries)
						{
							ManipulateAuth (tr ("Enter reason for rerequesting authorization from %1:"),
									entries,
									true,
									&IAuthable::RerequestAuth);
						}
					}));
		}

		if (const auto ihb = qobject_cast<IHaveBlacklists*> (acc->GetQObject ()))
			if (ihb->SupportsBlacklists ())
			{
				const auto block = new QAction (tr ("Blacklist..."), entry->GetQObject ());
				block->setProperty ("ActionIcon", "im-ban-user");
				Entry2Actions_ [entry] ["block"] = block;
				Action2Areas_ [block] << CLEAAContactListCtxtMenu;
				block->setProperty ("Azoth/EntryActor",
						QVariant::fromValue<EntryActor_f> ({
							[ihb] (const QList<ICLEntry*>& entries)
								{ ihb->SuggestToBlacklist (entries); }
						}));
			}

		auto notifyMenu = new QMenu (tr ("Notify when"));
		Entry2Actions_ [entry] ["notifywhen"] = notifyMenu->menuAction ();
		Action2Areas_ [notifyMenu->menuAction ()] << CLEAAContactListCtxtMenu
				<< CLEAATabCtxtMenu;
		if (entry->GetEntryType () != ICLEntry::EntryType::MUC)
		{
			notifyMenu->addAction (tr ("changes state"),
					this, SLOT (handleActionNotifyChangesState ()));
			notifyMenu->addAction (tr ("becomes online"),
					this, SLOT (handleActionNotifyBecomesOnline ()));
		}
		else
			notifyMenu->addAction (tr ("participant enters the room..."),
					this, SLOT (handleActionNotifyParticipantEnter ()));

		const auto accObj = entry->GetParentAccount ()->GetQObject ();
		if (qobject_cast<IHaveServerHistory*> (accObj))
		{
			auto openHistory = new QAction (tr ("Open server history..."), entry->GetQObject ());
			openHistory->setToolTip (tr ("View server history log with this contact"));
			openHistory->setProperty ("ActionIcon", "network-server-database");
			Entry2Actions_ [entry] ["view_server_history"] = openHistory;
			Action2Areas_ [openHistory] << CLEAAContactListCtxtMenu;
		}

#ifdef ENABLE_CRYPT
		if (qobject_cast<ISupportPGP*> (accObj))
		{
			QAction *manageGPG = new QAction (tr ("Manage PGP keys..."), entry->GetQObject ());
			manageGPG->setProperty ("ActionIcon", "document-encrypt");
			Entry2Actions_ [entry] ["managepgp"] = manageGPG;
			Action2Areas_ [manageGPG] << CLEAAContactListCtxtMenu;
		}
#endif

		if (qobject_cast<ISupportRIEX*> (accObj))
		{
			QAction *shareRIEX = new QAction (tr ("Share contacts..."), entry->GetQObject ());
			Entry2Actions_ [entry] ["shareRIEX"] = shareRIEX;
			Action2Areas_ [shareRIEX] << CLEAAContactListCtxtMenu;
		}

		if (entry->GetEntryType () != ICLEntry::EntryType::MUC)
		{
			const auto inviteTo = new QAction (tr ("Invite to a MUC..."), entry->GetQObject ());
			Entry2Actions_ [entry] ["inviteToMuc"] = inviteTo;
			Action2Areas_ [inviteTo] << CLEAAContactListCtxtMenu;

			const auto saveAvatar = new QAction (tr ("Save avatar..."), entry->GetQObject ());
			Entry2Actions_ [entry] ["saveAvatar"] = saveAvatar;
			Action2Areas_ [saveAvatar] << CLEAAContactListCtxtMenu;

			const auto vcard = new QAction (tr ("VCard"), entry->GetQObject ());
			vcard->setProperty ("ActionIcon", "text-x-vcard");
			Entry2Actions_ [entry] ["vcard"] = vcard;
			Action2Areas_ [vcard] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu
					<< CLEAAToolbar
					<< CLEAAChatCtxtMenu;
		}

		const auto perms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntryObject ());
		if (entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
		{
			if (perms)
			{
				const auto& possible = perms->GetPossiblePerms ();
				for (const QByteArray& permClass : possible.keys ())
				{
					QMenu *changeClass = new QMenu (perms->GetUserString (permClass));
					Entry2Actions_ [entry] [permClass] = changeClass->menuAction ();
					Action2Areas_ [changeClass->menuAction ()] << CLEAAContactListCtxtMenu
							<< CLEAAChatCtxtMenu
							<< CLEAATabCtxtMenu;

					const auto& possibles = possible [permClass];

					auto addPossible = [&possibles, perms, entry, permClass, this] (QMenu *menu, std::function<void (QList<ICLEntry*>, QAction*)> actor)
					{
						for (const QByteArray& perm : possibles)
						{
							QAction *permAct = menu->addAction (perms->GetUserString (perm),
									this,
									SLOT (handleActoredActionTriggered ()));
							permAct->setParent (entry->GetQObject ());
							permAct->setCheckable (true);
							permAct->setProperty ("Azoth/TargetPermClass", permClass);
							permAct->setProperty ("Azoth/TargetPerm", perm);

							auto fixedActor = [actor, permAct] (const QList<ICLEntry*>& entries)
									{ actor (entries, permAct); };
							permAct->setProperty ("Azoth/EntryActor",
									QVariant::fromValue<EntryActor_f> (MultiEntryActor_f (fixedActor)));
							connect (permAct,
									SIGNAL (triggered ()),
									this,
									SLOT (handleActoredActionTriggered ()),
									Qt::UniqueConnection);
						}
					};

					addPossible (changeClass,
							[] (const QList<ICLEntry*>& es, QAction *act)
								{ ChangePermMulti (act, es); });

					changeClass->addSeparator ();
					auto advanced = changeClass->addMenu (tr ("Advanced..."));
					advanced->setToolTip (tr ("Allows one to set advanced fields like "
							"reason or global flag"));

					addPossible (advanced,
							[] (const QList<ICLEntry*>& es, QAction *act)
								{ ChangePermAdvanced (act, es); });
				}

				QAction *sep = Util::CreateSeparator (entry->GetQObject ());
				Entry2Actions_ [entry] ["sep_afterroles"] = sep;
				Action2Areas_ [sep] << CLEAAContactListCtxtMenu;
			}

			QAction *addContact = new QAction (tr ("Add to contact list..."), entry->GetQObject ());
			addContact->setProperty ("ActionIcon", "list-add");
			Entry2Actions_ [entry] ["add_contact"] = addContact;
			Action2Areas_ [addContact] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu
					<< CLEAAChatCtxtMenu;

			QAction *copyId = new QAction (tr ("Copy ID"), entry->GetQObject ());
			copyId->setProperty ("ActionIcon", "edit-copy");
			Entry2Actions_ [entry] ["copy_muc_id"] = copyId;
			Action2Areas_ [copyId] << CLEAAContactListCtxtMenu
					<< CLEAAChatCtxtMenu;

			QAction *sep = Util::CreateSeparator (entry->GetQObject ());
			Entry2Actions_ [entry] ["sep_afterjid"] = sep;
			Action2Areas_ [sep] << CLEAAContactListCtxtMenu;
		}
		else if (entry->GetEntryType () == ICLEntry::EntryType::MUC)
		{
			auto sepBeforeMuc = Util::CreateSeparator (entry->GetQObject ());
			Entry2Actions_ [entry] ["sep_beforemuc"] = sepBeforeMuc;
			Action2Areas_ [sepBeforeMuc] << CLEAAContactListCtxtMenu;

			QAction *changeNick = new QAction (tr ("Change nickname..."), entry->GetQObject ());
			changeNick->setProperty ("ActionIcon", "user-properties");
			Entry2Actions_ [entry] ["changenick"] = changeNick;
			Action2Areas_ [changeNick] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu
					<< CLEAAToolbar;

			QAction *invite = new QAction (tr ("Invite..."), entry->GetQObject ());
			invite->setProperty ("ActionIcon", "azoth_invite");
			Entry2Actions_ [entry] ["invite"] = invite;
			Action2Areas_ [invite] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu;

			QAction *leave = new QAction (tr ("Leave"), entry->GetQObject ());
			leave->setProperty ("ActionIcon", "irc-close-channel");
			Entry2Actions_ [entry] ["leave"] = leave;
			Action2Areas_ [leave] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu
					<< CLEAAToolbar;
			sm->RegisterAction ("org.LeechCraft.Azoth.LeaveMUC", leave);

			QAction *reconnect = new QAction (tr ("Reconnect"), entry->GetQObject ());
			reconnect->setProperty ("ActionIcon", "view-refresh");
			Entry2Actions_ [entry] ["reconnect"] = reconnect;
			Action2Areas_ [reconnect] << CLEAAContactListCtxtMenu
					<< CLEAATabCtxtMenu;

			QAction *bookmarks = new QAction (tr ("Add to bookmarks"), entry->GetQObject ());
			bookmarks->setProperty ("ActionIcon", "bookmark-new");
			Entry2Actions_ [entry] ["addtobm"] = bookmarks;
			Action2Areas_ [bookmarks] << CLEAAContactListCtxtMenu
					<< CLEAAToolbar;

			QAction *userList = new QAction (tr ("MUC users..."), entry->GetQObject ());
			userList->setProperty ("ActionIcon", "system-users");
			userList->setShortcut (QString ("Ctrl+M"));
			Entry2Actions_ [entry] ["userslist"] = userList;
			Action2Areas_ [userList] << CLEAAToolbar;
			sm->RegisterAction ("org.LeechCraft.Azoth.MUCUsers", userList);

			if (qobject_cast<IConfigurableMUC*> (entry->GetQObject ()))
			{
				QAction *configureMUC = new QAction (tr ("Configure MUC..."), this);
				configureMUC->setProperty ("ActionIcon", "configure");
				Entry2Actions_ [entry] ["configuremuc"] = configureMUC;
				Action2Areas_ [configureMUC] << CLEAAContactListCtxtMenu
						<< CLEAAToolbar;
			}
		}
		else if (entry->GetEntryType () == ICLEntry::EntryType::UnauthEntry)
		{
			QAction *authorize = new QAction (tr ("Authorize"), entry->GetQObject ());
			Entry2Actions_ [entry] ["authorize"] = authorize;
			Action2Areas_ [authorize] << CLEAAContactListCtxtMenu;

			QAction *denyAuth = new QAction (tr ("Deny authorization"), entry->GetQObject ());
			Entry2Actions_ [entry] ["denyauth"] = denyAuth;
			Action2Areas_ [denyAuth] << CLEAAContactListCtxtMenu;
		}
		else if (entry->GetEntryType () == ICLEntry::EntryType::Chat)
		{
			QAction *remove = new QAction (tr ("Remove"), entry->GetQObject ());
			remove->setProperty ("ActionIcon", "list-remove");
			Entry2Actions_ [entry] ["remove"] = remove;
			Action2Areas_ [remove] << CLEAAContactListCtxtMenu;
		}

		QAction *sep = Util::CreateSeparator (entry->GetQObject ());
		Entry2Actions_ [entry] ["sep_afterinitiate"] = sep;
		Action2Areas_ [sep] << CLEAAContactListCtxtMenu;
		sep = Util::CreateSeparator (entry->GetQObject ());
		Entry2Actions_ [entry] ["sep_afterrostermodify"] = sep;
		Action2Areas_ [sep] << CLEAAContactListCtxtMenu;

		Entrifier { QVariant::fromValue<ICLEntry*> (entry) } (Entry2Actions_ [entry]);
	}

	namespace
	{
		void UpdatePermChangeState (QMenu *menu, IMUCPerms *mucPerms, QObject *entryObj, const QByteArray& permClass)
		{
			for (QAction *action : menu->actions ())
			{
				const QByteArray& perm = action->property ("Azoth/TargetPerm").toByteArray ();
				if (action->menu ())
					UpdatePermChangeState (action->menu (), mucPerms, entryObj, permClass);
				else if (!action->isSeparator ())
				{
					action->setEnabled (mucPerms->MayChangePerm (entryObj, permClass, perm));
					action->setChecked (mucPerms->GetPerms (entryObj) [permClass].contains (perm));
				}
			}
		}
	}

	void ActionsManager::UpdateActionsForEntry (ICLEntry *entry)
	{
		if (!entry)
			return;

		const auto advEntry = qobject_cast<IAdvancedCLEntry*> (entry->GetQObject ());
		const auto account = entry->GetParentAccount ();
		const bool isOnline = account->GetState ().State_ != SOffline;
		if (entry->GetEntryType () != ICLEntry::EntryType::MUC)
		{
			bool enableVCard = account->GetAccountFeatures () & IAccount::FCanViewContactsInfoInOffline ||
					isOnline;
			Entry2Actions_ [entry] ["vcard"]->setEnabled (enableVCard);

			Entry2Actions_ [entry] ["saveAvatar"]->setEnabled (AvatarsManager_->HasAvatar (entry->GetQObject ()));

			const auto& allEntries = account->GetCLEntries ();
			const auto hasMucs = std::any_of (allEntries.begin (), allEntries.end (),
					[] (QObject *entryObj)
					{
						return qobject_cast<ICLEntry*> (entryObj)->GetEntryType () == ICLEntry::EntryType::MUC;
					});

			Entry2Actions_ [entry] ["inviteToMuc"]->setEnabled (hasMucs);
		}

		Entry2Actions_ [entry] ["rename"]->setEnabled (entry->GetEntryFeatures () & ICLEntry::FSupportsRenames);

		if (const auto ihsh = qobject_cast<IHaveServerHistory*> (entry->GetParentAccount ()->GetQObject ()))
		{
			const bool supports = ihsh->HasFeature (ServerHistoryFeature::AccountSupportsHistory);
			Entry2Actions_ [entry] ["view_server_history"]->setEnabled (supports);
		}

		if (advEntry)
		{
			bool suppAtt = advEntry->GetAdvancedFeatures () & IAdvancedCLEntry::AFSupportsAttention;
			Entry2Actions_ [entry] ["drawattention"]->setEnabled (suppAtt);
		}

		if (entry->GetEntryType () == ICLEntry::EntryType::Chat)
		{
			Entry2Actions_ [entry] ["remove"]->setEnabled (isOnline);
			if (Entry2Actions_ [entry] ["authorization"])
				Entry2Actions_ [entry] ["authorization"]->setEnabled (isOnline);
		}

		IMUCEntry *thisMuc = qobject_cast<IMUCEntry*> (entry->GetQObject ());
		if (thisMuc)
			Entry2Actions_ [entry] ["invite"]->
					setEnabled (thisMuc->GetMUCFeatures () & IMUCEntry::MUCFCanInvite);

		const auto mucPerms = qobject_cast<IMUCPerms*> (entry->GetParentCLEntryObject ());
		if (entry->GetEntryType () == ICLEntry::EntryType::PrivateChat)
		{
			if (mucPerms)
			{
				const auto& possible = mucPerms->GetPossiblePerms ();
				QObject *entryObj = entry->GetQObject ();

				for (const auto& permClass : possible.keys ())
					UpdatePermChangeState (Entry2Actions_ [entry] [permClass]->menu (),
							mucPerms, entryObj, permClass);
			}

			const QString& realJid = GetMUCRealID (entry);
			Entry2Actions_ [entry] ["add_contact"]->setEnabled (!realJid.isEmpty ());
			Entry2Actions_ [entry] ["copy_muc_id"]->setEnabled (!realJid.isEmpty ());
		}
	}

	namespace
	{
		QList<ICLEntry*> GetEntriesFromAction (QAction *action)
		{
			if (const auto entry = action->property ("Azoth/Entry").value<ICLEntry*> ())
				return { entry };

			const auto& entriesVar = action->property ("Azoth/Entries");
			if (entriesVar.isValid ())
				return entriesVar.value<EntriesList_t> ();

			qWarning () << Q_FUNC_INFO
					<< "neither Entry nor Entries properties are set for"
					<< action->text ();
			return {};
		}
	}

	void ActionsManager::handleActoredActionTriggered ()
	{
		const auto action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		const auto& function = action->property ("Azoth/EntryActor").value<EntryActor_f> ();

		const auto& entries = GetEntriesFromAction (action);
		if (entries.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no entries for"
					<< action->text ();
			return;
		}

		Util::Visit (function,
				[&entries] (const SingleEntryActor_f& actor)
				{
					for (const auto entry : entries)
						actor (entry);
				},
				[&entries] (const MultiEntryActor_f& actor) { actor (entries); },
				[action] (const Util::Void&)
				{
					qWarning () << Q_FUNC_INFO
							<< "no function set for"
							<< action->text ();
				});
	}

	void ActionsManager::handleActionNotifyChangesState ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		auto entry = action->property ("Azoth/Entry").value<ICLEntry*> ();
		const auto& hrId = entry->GetHumanReadableID ();

		const auto& e = Util::MakeANRule (tr ("Notify when %1 changes state").arg (hrId),
				"org.LeechCraft.Azoth",
				AN::CatIM,
				{ AN::TypeIMStatusChange },
				AN::NotifyPersistent | AN::NotifyTransient | AN::NotifySingleShot,
				false,
				{
					{
						"org.LC.Plugins.Azoth.SourceID",
						ANStringFieldValue { entry->GetEntryID () }
					}
				});
		Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (e);
	}

	void ActionsManager::handleActionNotifyBecomesOnline ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		auto entry = action->property ("Azoth/Entry").value<ICLEntry*> ();
		const auto& hrId = entry->GetHumanReadableID ();

		const auto& e = Util::MakeANRule (tr ("Notify when %1 becomes online").arg (hrId),
				"org.LeechCraft.Azoth",
				AN::CatIM,
				{ AN::TypeIMStatusChange },
				AN::NotifyPersistent | AN::NotifyTransient | AN::NotifySingleShot,
				false,
				{
					{
						"org.LC.Plugins.Azoth.SourceID",
						ANStringFieldValue { entry->GetEntryID () }
					},
					{
						"org.LC.Plugins.Azoth.NewStatus",
						ANStringFieldValue
						{
							Core::Instance ().GetPluginProxy ()->StateToString (SOnline)
						}
					}
				});
		Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (e);
	}

	void ActionsManager::handleActionNotifyParticipantEnter ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
		{
			qWarning () << Q_FUNC_INFO
					<< sender ()
					<< "is not a QAction";
			return;
		}

		const auto& nickname = QInputDialog::getText (nullptr,
				"LeechCraft",
				tr ("Enter the nick of the participant to alert for:"));
		if (nickname.isEmpty ())
			return;

		auto entry = action->property ("Azoth/Entry").value<ICLEntry*> ();
		const auto& hrId = entry->GetHumanReadableID ();

		const auto& e = Util::MakeANRule (tr ("Notify when %1 joins %2")
					.arg (nickname)
					.arg (hrId),
				"org.LeechCraft.Azoth",
				AN::CatIM,
				{ AN::TypeIMStatusChange },
				AN::NotifyPersistent | AN::NotifyTransient | AN::NotifySingleShot,
				false,
				{
					{
						"org.LC.Plugins.Azoth.SourceName",
						ANStringFieldValue { nickname }
					},
					{
						"org.LC.Plugins.Azoth.ParentSourceID",
						ANStringFieldValue { entry->GetEntryID () }
					}
				});
		Core::Instance ().GetProxy ()->GetEntityManager ()->HandleEntity (e);
	}
}
}
