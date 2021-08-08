/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "rosenthal.h"
#include <QIcon>
#include <QApplication>
#include <QTextEdit>
#include <QContextMenuEvent>
#include <QMenu>
#include <QTranslator>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include "highlighter.h"

namespace LC
{
namespace Azoth
{
namespace Rosenthal
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Proxy_ = proxy;
		Util::InstallTranslator ("azoth_rosenthal");
	}

	void Plugin::SecondInit ()
	{
		const auto& providers = Proxy_->GetPluginsManager ()->
				GetAllCastableTo<ISpellCheckProvider*> ();
		for (const auto prov : providers)
			if ((Checker_ = prov->CreateSpellchecker ()))
				break;

		if (!Checker_)
			qWarning () << Q_FUNC_INFO
					<< "no spellchecker has been found, spell checking won't work";
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Rosenthal";
	}

	void Plugin::Release ()
	{
	}

	QString Plugin::GetName () const
	{
		return "Azoth Rosenthal";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Provides spellchecking for Azoth.");
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

	bool Plugin::eventFilter (QObject *obj, QEvent *event)
	{
		if (!Checker_)
			return QObject::eventFilter (obj, event);

		QPoint eventPos;
		if (event->type () == QEvent::ContextMenu)
			eventPos = static_cast<QContextMenuEvent*> (event)->pos ();
		else if (event->type () == QEvent::MouseButtonPress)
		{
			QMouseEvent *me = static_cast<QMouseEvent*> (event);
			if (me->buttons () & Qt::RightButton)
				eventPos = me->pos ();
			else
				return QObject::eventFilter (obj, event);
		}
		else
			return QObject::eventFilter (obj, event);

		QTextEdit *edit = qobject_cast<QTextEdit*> (obj);
		const QPoint& curPos = edit->mapToGlobal (eventPos);

		QTextCursor cur = edit->cursorForPosition (eventPos);
		cur.select (QTextCursor::WordUnderCursor);
		const auto& word = cur.selectedText ();
		QMenu *menu = edit->createStandardContextMenu (curPos);

		const auto& words = Checker_->GetPropositions (word);
		if (!words.isEmpty ())
		{
			QList<QAction*> acts;
			for (const auto& word : words)
			{
				QAction *act = new QAction (word, menu);
				acts << act;
				connect (act,
						SIGNAL (triggered ()),
						this,
						SLOT (handleCorrectionTriggered ()));
				act->setProperty ("TextEdit", QVariant::fromValue<QObject*> (edit));
				act->setProperty ("CursorPos", eventPos);
			}

			QAction *before = menu->actions ().first ();
			menu->insertActions (before, acts);
			menu->insertSeparator (before);
		}

		menu->exec (curPos);

		return true;
	}

	void Plugin::hookChatTabCreated (LC::IHookProxy_ptr,
			QObject *chatTab, QObject*, QWebEngineView*)
	{
		if (!Checker_)
			return;

		QTextEdit *edit = 0;
		QMetaObject::invokeMethod (chatTab,
				"getMsgEdit",
				Q_RETURN_ARG (QTextEdit*, edit));

		const auto hl = new Highlighter (Checker_, edit->document ());
		Highlighters_ << hl;
		connect (hl,
				SIGNAL (destroyed (QObject*)),
				this,
				SLOT (handleHighlighterDestroyed ()));

		edit->installEventFilter (this);
	}

	void Plugin::handleCorrectionTriggered ()
	{
		QAction *action = qobject_cast<QAction*> (sender ());
		if (!action)
			return;

		QTextEdit *edit = qobject_cast<QTextEdit*> (action->property ("TextEdit").value<QObject*> ());
		const QPoint& pos = action->property ("CursorPos").toPoint ();
		QTextCursor cur = edit->cursorForPosition (pos);
		cur.select (QTextCursor::WordUnderCursor);
		cur.deleteChar ();
		cur.insertText (action->text ());
	}

	void Plugin::handleHighlighterDestroyed ()
	{
		Highlighters_.removeAll (static_cast<Highlighter*> (sender ()));
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_rosenthal, LC::Azoth::Rosenthal::Plugin);
