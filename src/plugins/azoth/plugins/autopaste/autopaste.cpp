/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "autopaste.h"
#include <QIcon>
#include <QMessageBox>
#include <QTextEdit>
#include <QAction>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <util/sll/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/azoth/iclentry.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iaccount.h>
#include "xmlsettingsmanager.h"
#include "codepadservice.h"
#include "pastedialog.h"
#include "actionsstorage.h"

namespace LC::Azoth::Autopaste
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;

		ActionsStorage_ = new ActionsStorage { this };
		connect (ActionsStorage_,
				&ActionsStorage::pasteRequested,
				this,
				&Plugin::handlePasteRequested);

		XmlSettingsDialog_ = std::make_shared<Util::XmlSettingsDialog> ();
		XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothautopastesettings.xml");
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Autopaste";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Autopaste";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Detects long messages and suggests pasting them to a pastebin.");
	}

	QIcon Plugin::GetIcon () const
	{
		return Proxy_->GetIconThemeManager ()->GetPluginIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin" };
	}

	Util::XmlSettingsDialog_ptr Plugin::GetSettingsDialog () const
	{
		return XmlSettingsDialog_;
	}

	namespace
	{
		class IUserChoiceHandler
		{
		public:
			virtual bool ShouldAsk (QSettings&) const = 0;
			virtual void Rejected (QSettings&) = 0;
			virtual void Accepted (QSettings&) = 0;
		};

		using IUserChoiceHandler_ptr = std::shared_ptr<IUserChoiceHandler>;

		class NoOpChoiceHandler final : public IUserChoiceHandler
		{
		public:
			bool ShouldAsk (QSettings&) const override
			{
				return true;
			}

			void Rejected (QSettings&) override
			{
			}

			void Accepted (QSettings&) override
			{
			}
		};

		class CountingUserChoiceHandler final : public IUserChoiceHandler
		{
			const ICLEntry * const Entry_;
		public:
			CountingUserChoiceHandler (const ICLEntry *entry)
			: Entry_ { entry }
			{
			}

			bool ShouldAsk (QSettings& settings) const override
			{
				return !settings.value ("DontSuggest").toBool ();
			}

			void Rejected (QSettings& settings) override
			{
				if (settings.value ("DontDisable").toBool ())
					return;

				const auto nextRejectionCount = settings.value ("RejectionCount", 0).toInt () + 1;
				settings.setValue ("RejectionCount", nextRejectionCount);

				const auto threshold = 5;
				if (nextRejectionCount != threshold)
					return;

				if (QMessageBox::question (nullptr,
						"LeechCraft",
						Plugin::tr ("Do you want to disable autopasting for this contact (%1)?")
							.arg ("<em>" + Entry_->GetEntryName () + "</em>"),
						QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
				{
					settings.setValue ("DontSuggest", true);
					settings.setValue ("NameAtCancel", Entry_->GetEntryName ());
				}
				else
					settings.setValue ("DontDisable", true);
			}

			void Accepted (QSettings& settings) override
			{
				settings.setValue ("RejectionCount", 0);
			}
		};

		template<typename OkF, typename CancelF>
		void PerformPaste (ICLEntry *other, const QString& text,
				const ICoreProxy_ptr& proxy, const IUserChoiceHandler_ptr& handler,
				OkF okCont, CancelF cancelCont)
		{
			QSettings settings (QCoreApplication::organizationName (),
					QCoreApplication::applicationName () + "_Azoth_Autopaste");
			settings.beginGroup ("SavedChoices");
			settings.beginGroup (other->GetEntryID ());
			const auto guard = Util::MakeScopeGuard ([&settings]
					{
						settings.endGroup ();
						settings.endGroup ();
					});

			if (!handler->ShouldAsk (settings))
				return;

			PasteDialog dia;

			dia.SetCreatorName (settings.value ("Service").toString ());
			dia.SetHighlight (static_cast<Highlight> (settings.value ("Highlight").toInt ()));

			dia.exec ();

			switch (dia.GetChoice ())
			{
			case PasteDialog::Cancel:
				cancelCont ();
				break;
			case PasteDialog::No:
				handler->Rejected (settings);
				break;
			case PasteDialog::Yes:
			{
				auto service = dia.GetCreator () (other->GetQObject (), proxy);
				service->Paste ({ proxy->GetNetworkAccessManager (), text, dia.GetHighlight () });
				okCont ();

				handler->Accepted (settings);

				settings.setValue ("Service", dia.GetCreatorName ());
				settings.setValue ("Highlight", static_cast<int> (dia.GetHighlight ()));
				break;
			}
			}
		}
	}

	void Plugin::initPlugin (QObject *obj)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (obj);
	}

	void Plugin::hookMessageSendRequested (LC::IHookProxy_ptr proxy,
			QObject*, QObject *entry, int, QString)
	{
		ICLEntry *other = qobject_cast<ICLEntry*> (entry);
		if (!other)
		{
			qWarning () << Q_FUNC_INFO
				<< "unable to cast"
				<< entry
				<< "to ICLEntry";
			return;
		}

		const auto& text = proxy->GetValue ("text").toString ();

		const int maxLines = XmlSettingsManager::Instance ()
				.property ("LineCount").toInt ();
		const int maxSymbols = XmlSettingsManager::Instance ()
				.property ("SymbolCount").toInt ();
		if (text.size () < maxSymbols &&
				text.count ('\n') + 1 < maxLines)
			return;

		QByteArray propName;
		switch (other->GetEntryType ())
		{
		case ICLEntry::EntryType::Chat:
			propName = "EnableForNormalChats";
			break;
		case ICLEntry::EntryType::MUC:
			propName = "EnableForMUCChats";
			break;
		case ICLEntry::EntryType::PrivateChat:
			propName = "EnableForPrivateChats";
			break;
		default:
			return;
		}

		if (!XmlSettingsManager::Instance ().property (propName).toBool ())
			return;

		PerformPaste (other, text, Proxy_, std::make_shared<CountingUserChoiceHandler> (other),
				[proxy] { proxy->CancelDefault (); },
				[proxy]
				{
					proxy->CancelDefault ();
					proxy->SetValue ("PreserveMessageEdit", true);
				});
	}

	void Plugin::hookEntryActionAreasRequested (IHookProxy_ptr proxy, QObject *action, QObject*)
	{
		const auto& ours = ActionsStorage_->GetActionAreas (action);
		if (ours.isEmpty ())
			return;

		proxy->SetReturnValue (proxy->GetReturnValue ().toStringList () + ours);
	}

	void Plugin::hookEntryActionsRequested (IHookProxy_ptr proxy, QObject *entry)
	{
		const auto& actions = ActionsStorage_->GetEntryActions (entry);
		if (actions.isEmpty ())
			return;

		auto list = proxy->GetReturnValue ().toList ();
		for (const auto action : actions)
			list << QVariant::fromValue<QObject*> (action);
		proxy->SetReturnValue (list);
	}

	void Plugin::handlePasteRequested (QObject *entryObj)
	{
		const auto entry = qobject_cast<ICLEntry*> (entryObj);
		const auto tab = AzothProxy_->FindOpenedChat (entry->GetEntryID (),
				entry->GetParentAccount ()->GetAccountID ());
		if (!tab)
		{
			qWarning () << Q_FUNC_INFO
					<< "no tab for"
					<< entry
					<< entry->GetEntryID ()
					<< entry->GetHumanReadableID ();
			return;
		}

		QTextEdit *edit = nullptr;
		QMetaObject::invokeMethod (tab, "getMsgEdit", Q_RETURN_ARG (QTextEdit*, edit));
		if (!edit)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot get message edit";
			return;
		}

		const auto& text = edit->toPlainText ();
		if (text.isEmpty ())
			return;

		PerformPaste (entry, text, Proxy_, std::make_shared<NoOpChoiceHandler> (),
				[edit] { edit->clear (); },
				[] {});
	}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_autopaste, LC::Azoth::Autopaste::Plugin);
