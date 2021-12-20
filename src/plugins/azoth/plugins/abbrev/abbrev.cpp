/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "abbrev.h"
#include <QIcon>
#include <util/util.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <interfaces/azoth/iproxyobject.h>
#include <interfaces/azoth/iclentry.h>
#include "abbrevsmanager.h"
#include "shortcutsmanager.h"

namespace LC::Azoth::Abbrev
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		qRegisterMetaType<Abbreviation> ("LC::Azoth::Abbrev::Abbreviation");
		qRegisterMetaType<QList<Abbreviation>> ("QList<LC::Azoth::Abbrev::Abbreviation>");
		qRegisterMetaTypeStreamOperators<Abbreviation> ();
		qRegisterMetaTypeStreamOperators<QList<Abbreviation>> ();

		Util::InstallTranslator (QStringLiteral ("azoth_abbrev"));

		Manager_ = std::make_shared<AbbrevsManager> ();

		Commands_.append ({
				{ "/abbrev" },
				[this] (ICLEntry*, const QString& text) -> StringCommandResult
				{
					const auto& pattern = text.section (' ', 1, 1).trimmed ();
					const auto& expansion = text.section (' ', 2).trimmed ();
					Manager_->Add ({ pattern, expansion });
					return
					{
						true,
						tr ("Pattern %1 has been added successfully.")
								.arg ("<em>" + pattern + "</em>")
					};
				},
				tr ("Adds a new abbreviation to the list of abbreviations."),
				tr ("Usage: @/abbrev@ _pattern_ _text_\n\n"
					"Adds a new _pattern_ that expands to the given _text_, which can span "
					"multiple lines.\n\n"
					"@/listabbrevs@ lists all available abbreviations and @/unabbrev@ allows "
					"removing them.")
			});
		Commands_.append ({
				{ "/listabbrevs" },
				[this] (ICLEntry *entry, const QString&) -> bool
				{
					ListAbbrevs (entry);
					return true;
				},
				tr ("Lists all abbreviations that were previously added."),
				{}
			});
		Commands_.append ({
				{ "/unabbrev" },
				[this] (ICLEntry*, const QString& text) -> StringCommandResult
				{
					const auto& pattern = text.section (' ', 1).trimmed ();
					RemoveAbbrev (pattern);
					return
					{
						true,
						tr ("Pattern %1 has been removed successfully.")
								.arg ("<em>" + pattern + "</em>")
					};
				},
				tr ("Removes a previously added abbreviation."),
				tr ("Usage: @/unabbrev@ <_pattern_|_index_>\n\n"
					"Removes a previously added abbrevation either by its _pattern_ or by its "
					"_index_ in the list returned by @/listabbrevs@.")
			});

		ShortcutsMgr_ = new ShortcutsManager { Manager_.get (), this };
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Abbrev";
	}

	void Plugin::Release ()
	{
		Manager_.reset ();
	}

	QString Plugin::GetName () const
	{
		return QStringLiteral ("Azoth Abbrev");
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides support for automatically expanding abbreviations for Azoth.");
	}

	QIcon Plugin::GetIcon () const
	{
		return {};
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		return { "org.LeechCraft.Plugins.Azoth.Plugins.IGeneralPlugin" };
	}

	QMap<QString, ActionInfo> Plugin::GetActionInfo () const
	{
		return ShortcutsMgr_->GetActionInfo ();
	}

	void Plugin::SetShortcut (const QString& id, const QKeySequences_t& seqs)
	{
		ShortcutsMgr_->SetShortcut (id, seqs);
	}

	StaticCommands_t Plugin::GetStaticCommands (ICLEntry*)
	{
		return Commands_;
	}

	void Plugin::ListAbbrevs (ICLEntry *entry)
	{
		const auto& abbrevs = Util::Map (Manager_->List (),
				[] (const auto& abbrev) noexcept
				{
					return u"%1 → %2"_qsv
							.arg (abbrev.Pattern_,
								  abbrev.Expansion_);
				});

		const auto& text = tr ("%n abbreviation(s):", nullptr, abbrevs.size ()) +
				"<ol><li>" + abbrevs.join (u"</li><li>"_qsv) + "</li></ol>";

		const auto entryObj = entry->GetQObject ();
		const auto msgObj = AzothProxy_->CreateCoreMessage (text,
				QDateTime::currentDateTime (),
				IMessage::Type::ServiceMessage,
				IMessage::Direction::In,
				entryObj,
				entryObj);
		const auto msg = qobject_cast<IMessage*> (msgObj);
		msg->Store ();
	}

	void Plugin::RemoveAbbrev (const QString& text)
	{
		bool ok = false;
		const auto idx = text.toInt (&ok);
		if (ok)
		{
			Manager_->Remove (idx - 1);
			return;
		}

		const auto& list = Manager_->List ();
		const auto pos = std::find_if (list.begin (), list.end (),
				[&text] (const Abbreviation& abbrev) { return abbrev.Pattern_ == text; });
		if (pos == list.end ())
			throw CommandException { tr ("Unable to find abbreviation %1.")
					.arg ("<em>" + text + "</em>") };

		Manager_->Remove (std::distance (list.begin (), pos));
	}

	void Plugin::initPlugin (QObject *proxyObj)
	{
		AzothProxy_ = qobject_cast<IProxyObject*> (proxyObj);
	}

	void Plugin::hookChatTabCreated (IHookProxy_ptr, QObject *chatTab, QObject*, QWebEngineView*)
	{
		ShortcutsMgr_->HandleTab (qobject_cast<QWidget*> (chatTab));
	}

	void Plugin::hookMessageSendRequested (LC::IHookProxy_ptr proxy,
			QObject*, QObject *entryObj, int, QString)
	{
		const auto& text = proxy->GetValue ("text").toString ();

		try
		{
			const auto& newText = Manager_->Process (text);
			if (text != newText)
				proxy->SetValue ("text", newText);
		}
		catch (const CommandException& e)
		{
			const auto msgObj = AzothProxy_->CreateCoreMessage (e.GetError (),
					QDateTime::currentDateTime (),
					IMessage::Type::ServiceMessage,
					IMessage::Direction::In,
					entryObj,
					entryObj);
			const auto msg = qobject_cast<IMessage*> (msgObj);
			msg->Store ();
		}
	}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_abbrev, LC::Azoth::Abbrev::Plugin);
