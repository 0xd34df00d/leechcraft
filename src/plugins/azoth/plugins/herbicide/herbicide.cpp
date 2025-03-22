/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "herbicide.h"
#include <QIcon>
#include <QAction>
#include <QSettings>
#include <QCoreApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <util/sll/qtutil.h>
#include <util/sll/slotclosure.h>
#include <util/xsd/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/imessage.h>
#include <interfaces/azoth/iaccount.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "xmlsettingsmanager.h"
#include "confwidget.h"
#include "logger.h"
#include "listsholder.h"

namespace LC
{
namespace Azoth
{
namespace Herbicide
{
	namespace
	{
		bool HasSpecificSettings (IAccount *acc)
		{
			QSettings settings
			{
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Herbicide"
			};

			return settings.childGroups ().contains (acc->GetAccountID ());
		}

		QVariant GetAccountProperty (IAccount *acc, const QByteArray& name)
		{
			QSettings settings
			{
				QCoreApplication::organizationName (),
				QCoreApplication::applicationName () + "_Azoth_Herbicide"
			};

			const auto& accId = acc ?
					acc->GetAccountID () :
					QByteArray {};
			if (!acc || !settings.childGroups ().contains (accId))
				return settings.value (name);

			settings.beginGroup (accId);
			const auto& value = settings.value (name);
			settings.endGroup ();

			return value;
		}

		QString GetQuestion (IAccount *acc)
		{
			return GetAccountProperty (acc, "Question").toString ();
		}

		QStringList GetAnswers (IAccount *acc)
		{
			return GetAccountProperty (acc, "Answers").toStringList ();
		}
	}

	void Plugin::Init (ICoreProxy_ptr)
	{
		SettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothherbicidesettings.xml");

		const auto confWidget = new ConfWidget (&XmlSettingsManager::Instance ());
		SettingsDialog_->SetCustomWidget ("ConfWidget", confWidget);

		Logger_ = new Logger;

		ListsHolder_ = std::make_shared<ListsHolder> (&GetAccountProperty);

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this] { ListsHolder_->ReloadLists (nullptr); },
			confWidget,
			SIGNAL (listsChanged ()),
			this
		};
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Herbicide";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Herbicide";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("A simple antispam plugin for Azoth.");
	}

	QIcon Plugin::GetIcon () const
	{
		return GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin";
		return result;
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return SettingsDialog_;
	}

	QList<QAction*> Plugin::CreateActions (IAccount *acc)
	{
		QList<QAction*> result;

		const auto configAction = new QAction { GetIcon (), tr ("Configure antispam settings..."), this };

		new Util::SlotClosure<Util::NoDeletePolicy>
		{
			[this, acc] { ShowAccountAntispamConfig (acc); },
			configAction,
			SIGNAL (triggered ()),
			configAction
		};

		result << configAction;

		if (HasSpecificSettings (acc))
		{
			const auto removeAction = new QAction { tr ("Remove account-specific antispam settings"), this };

			new Util::SlotClosure<Util::NoDeletePolicy>
			{
				[this, acc]
				{
					QSettings settings
					{
						QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_Azoth_Herbicide"
					};

					settings.remove (acc->GetAccountID ());
					ListsHolder_->ReloadLists (acc);
				},
				removeAction,
				SIGNAL (triggered ()),
				removeAction
			};

			result << removeAction;
		}

		return result;
	}

	bool Plugin::IsConfValid (IAccount *acc) const
	{
		if (!GetAccountProperty (acc, "EnableQuest").toBool ())
			return false;

		return !GetQuestion (acc).isEmpty () && !GetAnswers (acc).isEmpty ();
	}

	bool Plugin::IsEntryAllowed (QObject *entryObj) const
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		if (!entry)
			return true;

		if ((entry->GetEntryFeatures () & ICLEntry::FMaskLongetivity) == ICLEntry::FPermanentEntry)
		{
			Logger_->LogEvent (Logger::Event::Granted, entry, "entry is permanent");
			return true;
		}

		if (AllowedEntries_.contains (entryObj))
		{
			Logger_->LogEvent (Logger::Event::Granted, entry, "entry has been previously allowed");
			return true;
		}

		const auto& id = entry->GetHumanReadableID ();

		const auto checkRxList = [&id] (const QSet<QRegularExpression>& rxList)
		{
			return std::ranges::any_of (rxList,
					[&id] (const QRegularExpression& rx) { return rx.match (id).hasMatch (); });
		};

		const auto acc = entry->GetParentAccount ();
		if (checkRxList (ListsHolder_->GetWhitelist (acc)))
		{
			Logger_->LogEvent (Logger::Event::Granted, entry, "entry is in the whitelist");
			return true;
		}

		if (GetAccountProperty (entry->GetParentAccount (), "AskOnlyBL").toBool ())
		{
			const auto isBlacklisted = checkRxList (ListsHolder_->GetBlacklist (acc));
			if (isBlacklisted)
				Logger_->LogEvent (Logger::Event::Denied, entry, "entry is in the blacklist");
			else
				Logger_->LogEvent (Logger::Event::Granted, entry, "entry is not in the blacklist and blacklist-only mode is enabled");
			return !isBlacklisted;
		}

		Logger_->LogEvent (Logger::Event::Denied, entry, "fallback case");

		return false;
	}

	void Plugin::ChallengeEntry (IHookProxy_ptr proxy, QObject *entryObj)
	{
		auto entry = qobject_cast<ICLEntry*> (entryObj);
		AskedEntries_ << entryObj;

		auto greeting = GetAccountProperty (entry->GetParentAccount (), "QuestPrefix").toString ();
		if (!greeting.isEmpty ())
			greeting += "\n";
		const auto& text = greeting + GetQuestion (entry->GetParentAccount ());

		Logger_->LogEvent (Logger::Event::Challenged, entry, text);

		const auto msg = entry->CreateMessage (IMessage::Type::ChatMessage, QString (), text);
		OurMessages_ << msg;
		msg->Send ();

		proxy->CancelDefault ();
	}

	void Plugin::GreetEntry (QObject *entryObj)
	{
		AllowedEntries_ << entryObj;

		AskedEntries_.remove (entryObj);

		auto entry = qobject_cast<ICLEntry*> (entryObj);

		const auto& text = GetAccountProperty (entry->GetParentAccount (), "QuestSuccessReply").toString ();
		const auto msg = entry->CreateMessage (IMessage::Type::ChatMessage, QString (), text);
		OurMessages_ << msg;
		msg->Send ();

		if (DeniedAuth_.contains (entryObj))
			QMetaObject::invokeMethod (entry->GetParentAccount ()->GetQObject (),
					"authorizationRequested",
					Q_ARG (QObject*, entryObj),
					Q_ARG (QString, DeniedAuth_.take (entryObj)));
	}

	namespace
	{
		class AccountSettingsManager : public Util::BaseSettingsManager
		{
			const QByteArray GroupName_;
		public:
			AccountSettingsManager (IAccount *acc, QObject *parent)
			: Util::BaseSettingsManager { parent }
			, GroupName_ { acc->GetAccountID () }
			{
				Util::BaseSettingsManager::Init ();
			}
		protected:
			QSettings_ptr MakeSettings () const override
			{
				return Util::MakeGroupSettings ("Azoth_Herbicide"_qs, GroupName_);
			}
		};
	}

	void Plugin::ShowAccountAntispamConfig (IAccount *acc)
	{
		auto dia = new QDialog;
		dia->setLayout (new QVBoxLayout);

		auto xsd = new Util::XmlSettingsDialog;
		const auto accsm = new AccountSettingsManager { acc, xsd };

		xsd->RegisterObject (accsm, "azothherbicidesettings.xml");
		auto confWidget = new ConfWidget (accsm);
		xsd->SetCustomWidget ("ConfWidget", confWidget);

		new Util::SlotClosure<Util::DeleteLaterPolicy>
		{
			[this, acc] { ListsHolder_->ReloadLists (acc); },
			confWidget,
			SIGNAL (listsChanged ()),
			this
		};

		dia->layout ()->addWidget (xsd->GetWidget ());

		auto buttons = new QDialogButtonBox { QDialogButtonBox::Ok | QDialogButtonBox::Cancel };
		dia->layout ()->addWidget (buttons);

		connect (buttons,
				SIGNAL (accepted ()),
				xsd,
				SLOT (accept ()));
		connect (buttons,
				SIGNAL (accepted ()),
				dia,
				SLOT (accept ()));
		connect (buttons,
				SIGNAL (rejected ()),
				dia,
				SLOT (reject ()));

		dia->setAttribute (Qt::WA_DeleteOnClose);
		dia->open ();
		connect (dia,
				SIGNAL (finished (int)),
				xsd,
				SLOT (deleteLater ()));
	}

	void Plugin::hookGotAuthRequest (IHookProxy_ptr proxy, QObject *entry, QString msg)
	{
		const auto acc = qobject_cast<ICLEntry*> (entry)->GetParentAccount ();
		if (!IsConfValid (acc))
			return;

		if (!GetAccountProperty (acc, "EnableForAuths").toBool ())
			return;

		if (IsEntryAllowed (entry))
			return;

		if (!AskedEntries_.contains (entry))
		{
			ChallengeEntry (proxy, entry);
			DeniedAuth_ [entry] = msg;
		}
	}

	void Plugin::hookGotMessage (LC::IHookProxy_ptr proxy,
				QObject *message)
	{
		const auto msg = qobject_cast<IMessage*> (message);
		if (!msg)
		{
			qWarning () << Q_FUNC_INFO
					<< message
					<< "doesn't implement IMessage";
			return;
		}

		if (OurMessages_.contains (msg))
		{
			OurMessages_.remove (msg);
			proxy->CancelDefault ();
			return;
		}

		if (msg->GetMessageType () != IMessage::Type::ChatMessage)
			return;

		const auto entryObj = msg->OtherPart ();
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto acc = entry->GetParentAccount ();

		if (!IsConfValid (acc))
			return;

		if (IsEntryAllowed (entryObj))
			return;

		if (!AskedEntries_.contains (entryObj))
			ChallengeEntry (proxy, entryObj);
		else if (GetAnswers (acc).contains (msg->GetBody ().toLower ()))
		{
			Logger_->LogEvent (Logger::Event::Succeeded, entry, msg->GetBody ());
			GreetEntry (entryObj);
		}
		else
		{
			Logger_->LogEvent (Logger::Event::Failed, entry, msg->GetBody ());

			const auto& text = GetAccountProperty (acc, "QuestFailureReply").toString ();
			const auto msg = entry->CreateMessage (IMessage::Type::ChatMessage, QString (), text);
			OurMessages_ << msg;
			msg->Send ();

			proxy->CancelDefault ();
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_herbicide, LC::Azoth::Herbicide::Plugin);
