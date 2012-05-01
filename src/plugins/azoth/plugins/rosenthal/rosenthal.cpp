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

#include "rosenthal.h"
#include <QIcon>
#include <QApplication>
#include <QTextEdit>
#include <QContextMenuEvent>
#include <QMenu>
#include <QTextCodec>
#include <QTranslator>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "hunspell/hunspell.hxx"
#include "highlighter.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Rosenthal
{
	void Plugin::Init (ICoreProxy_ptr)
	{
		Translator_.reset (Util::InstallTranslator ("azoth_rosenthal"));

		SettingsDialog_.reset (new Util::XmlSettingsDialog);
		SettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
				"azothrosenthalsettings.xml");

		XmlSettingsManager::Instance ().RegisterObject ("CustomLocales",
				this, "handleCustomLocalesChanged");

		ReinitHunspell ();
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Azoth.Rosenthal";
	}

	void Plugin::Release ()
	{
		Hunspell_.reset ();
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
		return QIcon (":/plugins/azoth/plugins/rosenthal/resources/images/rosenthal.svg");
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

	bool Plugin::eventFilter (QObject *obj, QEvent *event)
	{
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
		QString word = cur.block ().text ();
		const int pos = cur.columnNumber ();
		const int end = word.indexOf (QRegExp("\\W+"), pos);
		const int begin = word.lastIndexOf (QRegExp("\\W+"), pos);
		word = word.mid (begin + 1, end - begin - 1);

		QMenu *menu = edit->createStandardContextMenu (curPos);

		const QStringList& words = GetPropositions (word);
		if (!words.isEmpty ())
		{
			QList<QAction*> acts;
			Q_FOREACH (const QString& word, words)
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

	void Plugin::ReinitHunspell ()
	{
		const QString& userSetting = XmlSettingsManager::Instance ()
				.property ("CustomLocales").toString ();
		const QStringList& userLocales = userSetting.split (' ', QString::SkipEmptyParts);

		const QString& locale = userLocales.value (0, Util::GetLocaleName ());

		QString base;
		QStringList candidates (Util::CreateIfNotExists ("data/dicts/myspell/").absolutePath ());
#ifdef Q_OS_WIN32
		candidates << qApp->applicationDirPath () + "/myspell/";
#else
		candidates << "/usr/local/share/myspell/"
				<< "/usr/share/myspell/"
				<< "/usr/local/share/myspell/dicts/"
				<< "/usr/share/myspell/dicts/"
				<< "/usr/local/share/hunspell/"
				<< "/usr/share/hunspell/";
#endif
		Q_FOREACH (base, candidates)
			if (QFile::exists (base + locale + ".aff"))
				break;

		QByteArray baBase = (base + locale).toLatin1 ();
		Hunspell_.reset (new Hunspell (baBase + ".aff", baBase + ".dic"));

		if (!locale.startsWith ("en_"))
			Hunspell_->add_dic ((base + "en_GB.dic").toLatin1 ());

		if (userLocales.size () > 1)
			Q_FOREACH (const QString& loc, userLocales)
			{
				if (loc == locale ||
						loc == "en_GB")
					continue;

				Hunspell_->add_dic ((base + loc + ".dic").toLatin1 ());
			}

		Q_FOREACH (Highlighter *hl, Highlighters_)
			hl->UpdateHunspell (Hunspell_);
	}

	QStringList Plugin::GetPropositions (const QString& word)
	{
		QTextCodec *codec = QTextCodec::codecForName (Hunspell_->get_dic_encoding ());
		const QByteArray& encoded = codec->fromUnicode (word);
		if (Hunspell_->spell (encoded.data ()))
			return QStringList ();

		char **wlist = 0;
		const int ns = Hunspell_->suggest (&wlist, encoded.data ());
		if (!ns || !wlist)
			return QStringList ();

		QStringList result;
		for (int i = 0; i < std::min (ns, 10); ++i)
			result << codec->toUnicode (wlist [i]);
		Hunspell_->free_list (&wlist, ns);

		return result;
	}

	void Plugin::hookChatTabCreated (LeechCraft::IHookProxy_ptr,
			QObject *chatTab, QObject*, QWebView*)
	{
		QTextEdit *edit;
		QMetaObject::invokeMethod (chatTab,
				"getMsgEdit",
				Q_RETURN_ARG (QTextEdit*, edit));

		Highlighter *hl = new Highlighter (Hunspell_, edit->document ());
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

	void Plugin::handleCustomLocalesChanged ()
	{
		ReinitHunspell ();
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_azoth_rosenthal, LeechCraft::Azoth::Rosenthal::Plugin);
