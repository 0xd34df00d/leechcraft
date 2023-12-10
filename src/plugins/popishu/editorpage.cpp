/**********************************************************************
1 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "editorpage.h"
#include <iostream>
#include <algorithm>
#include <QToolBar>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QMenu>
#include <QToolButton>
#include <QFileInfo>
#include <QUrl>
#include <QBuffer>
#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexercmake.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerdiff.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexermatlab.h>
#include <Qsci/qscilexeroctave.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexertex.h>
#include <Qsci/qscilexerxml.h>
#include <util/util.h>
#include <util/xpc/util.h>
#include <util/sll/prelude.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/ientitymanager.h>
#include "xmlsettingsmanager.h"
#include "replacedialog.h"

Q_DECLARE_METATYPE (QObject**);

namespace LC
{
namespace Popishu
{
	EditorPage::EditorPage (const ICoreProxy_ptr& proxy, const TabClassInfo& tc, QObject *parentPlugin)
	: TC_ (tc)
	, ParentPlugin_ (parentPlugin)
	, Proxy_ (proxy)
	, Toolbar_ (new QToolBar)
	{
		SetupDefPairs ();

		Ui_.setupUi (this);

		Toolbar_->addAction (Ui_.ActionNew_);
		Toolbar_->addSeparator ();

		RecentFilesMenu_ = new QMenu (tr ("Recent files"));
		RestoreRecentFiles ();

		auto openButton = new QToolButton ();
		openButton->setDefaultAction (Ui_.ActionOpen_);
		openButton->setMenu (RecentFilesMenu_);
		openButton->setPopupMode (QToolButton::MenuButtonPopup);

		Toolbar_->addWidget (openButton);

		Toolbar_->addAction (Ui_.ActionSave_);
		Toolbar_->addAction (Ui_.ActionSaveAs_);

		Toolbar_->addSeparator ();

		Toolbar_->addAction (Ui_.ActionReplace_);

		Ui_.TextEditor_->setAutoIndent (true);
		Ui_.TextEditor_->setUtf8 (true);
		Ui_.TextEditor_->setCaretLineBackgroundColor (palette ().color (QPalette::AlternateBase));

		DoctypeMenu_ = new QMenu (tr ("Document type"));
		DoctypeMenu_->addAction ("Bash")->setCheckable (true);
		DoctypeMenu_->addAction ("CMake")->setCheckable (true);
		DoctypeMenu_->addAction ("C++")->setCheckable (true);
		DoctypeMenu_->addAction ("CSS")->setCheckable (true);
		DoctypeMenu_->addAction ("Diff")->setCheckable (true);
		DoctypeMenu_->addAction ("HTML")->setCheckable (true);
		DoctypeMenu_->addAction ("JavaScript")->setCheckable (true);
		DoctypeMenu_->addAction ("MatLab")->setCheckable (true);
		DoctypeMenu_->addAction ("Octave")->setCheckable (true);
		DoctypeMenu_->addAction ("Python")->setCheckable (true);
		DoctypeMenu_->addAction ("Ruby")->setCheckable (true);
		DoctypeMenu_->addAction ("SQL")->setCheckable (true);
		DoctypeMenu_->addAction ("TeX")->setCheckable (true);
		DoctypeMenu_->addAction ("XML")->setCheckable (true);
		connect (DoctypeMenu_,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (selectDoctype (QAction*)));

		auto viewAction = new QAction (tr ("View"), this);
		viewAction->setProperty ("ActionIcon", "view-choose");

		auto viewMenu = new QMenu ();

		auto viewButton = new QToolButton ();
		viewButton->setDefaultAction (viewAction);
		viewButton->setMenu (viewMenu);
		viewButton->setPopupMode (QToolButton::InstantPopup);
		Toolbar_->addWidget (viewButton);

		QMenu *wsVis = new QMenu (tr ("Whitespace visibility"));
		wsVis->addAction (Ui_.ActionWSInvisible_);
		wsVis->addAction (Ui_.ActionWSVisible_);
		wsVis->addAction (Ui_.ActionWSVisibleAfterIndent_);
		GroupActions (wsVis->actions ());

		QMenu *wrapMode = new QMenu (tr ("Wrapping mode"));
		wrapMode->addAction (Ui_.ActionWrapNone_);
		wrapMode->addAction (Ui_.ActionWrapWords_);
		wrapMode->addAction (Ui_.ActionWrapCharacters_);
		GroupActions (wrapMode->actions ());

		viewMenu->addActions ({
				Ui_.ActionEnableFolding_,
				Ui_.ActionAutoIndent_,
				Ui_.ActionShowLineNumbers_,
				Util::CreateSeparator (this),
				DoctypeMenu_->menuAction (),
				wsVis->menuAction (),
				wrapMode->menuAction (),
				Util::CreateSeparator (this),
				Ui_.ActionShowEOL_,
				Ui_.ActionShowCaretLine_
			});

		connect (this,
				SIGNAL (languageChanged (const QString&)),
				this,
				SLOT (checkProperDoctypeAction (const QString&)));
		connect (this,
				SIGNAL (languageChanged (const QString&)),
				this,
				SLOT (checkInterpreters (const QString&)));

		connect (Ui_.ActionShowEOL_,
				SIGNAL (toggled (bool)),
				Ui_.TextEditor_,
				SLOT (setEolVisibility (bool)));
		connect (Ui_.ActionShowCaretLine_,
				SIGNAL (toggled (bool)),
				Ui_.TextEditor_,
				SLOT (setCaretLineVisible (bool)));
		connect (Ui_.ActionAutoIndent_,
				SIGNAL (toggled (bool)),
				Ui_.TextEditor_,
				SLOT (setAutoIndent (bool)));
		Ui_.TextEditor_->setCaretLineVisible (true);

		Ui_.TextEditor_->setAutoCompletionThreshold (1);
		on_ActionEnableFolding__toggled (Ui_.ActionEnableFolding_->isChecked ());

		Ui_.Inject_->setEnabled (true);
		Ui_.Release_->setEnabled (false);

		XmlSettingsManager::Instance ().RegisterObject ("MonoFont", this, "handleMonoFontChanged");

		QList<QByteArray> visualWrapPrefs;
		visualWrapPrefs << "EndLineFlag"
				<< "StartLineFlag"
				<< "WrappedIndent";
		XmlSettingsManager::Instance ().RegisterObject (visualWrapPrefs, this, "handleVisualWrapFlags");
		handleVisualWrapFlags ();

		QList<QByteArray> otherPrefs;
		otherPrefs << "TabWidget"
				<< "IdentationWidth";
		XmlSettingsManager::Instance ().RegisterObject (otherPrefs, this, "handleOtherPrefs");
		handleOtherPrefs ();

		ShowConsole (false);

		Ui_.ActionWrapWords_->trigger ();
		Ui_.ActionShowLineNumbers_->trigger ();
	}

	EditorPage::~EditorPage ()
	{
		if (WrappedObject_)
			Proxy_->GetPluginsManager ()->ReleasePlugin (WrappedObject_);
	}

	void EditorPage::Remove ()
	{
		if (Modified_ && !TemporaryDocument_)
		{
			QString name = QFileInfo (Filename_).fileName ();
			if (name.isEmpty ())
				name = tr ("Untitled");

			QMessageBox::StandardButton res =
					QMessageBox::question (this,
							"LeechCraft",
							tr ("The document <em>%1</em> is modified. "
								"Do you want to save it now?")
								.arg (name),
							QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
			if (res == QMessageBox::Cancel)
				return;
			else if (res == QMessageBox::Yes)
				on_ActionSave__triggered ();
		}

		emit removeTab ();
		deleteLater ();
	}

	QToolBar* EditorPage::GetToolBar () const
	{
		return Toolbar_.get ();
	}

	QObject* EditorPage::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	TabClassInfo EditorPage::GetTabClassInfo () const
	{
		return TC_;
	}

	QByteArray EditorPage::GetTabRecoverData () const
	{
		QByteArray result;

		{
			QDataStream ostr (&result, QIODevice::WriteOnly);
			ostr << QByteArray { "EditorPage/1" }
					<< Filename_;

			ostr << (Modified_ ? Ui_.TextEditor_->text () : QString ());

			qint32 line = 0, index = 0;
			Ui_.TextEditor_->getCursorPosition (&line, &index);

			ostr << line << index;
		}

		return result;
	}

	QIcon EditorPage::GetTabRecoverIcon () const
	{
		return GetTabClassInfo ().Icon_;
	}

	QString EditorPage::GetTabRecoverName () const
	{
		return Filename_.isEmpty () ?
				"Popishu" :
				QFileInfo { Filename_ }.fileName ();
	}

	void EditorPage::RestoreState (QDataStream& istr)
	{
		istr >> Filename_;
		Open (Filename_);

		QString text;
		istr >> text;
		Modified_ = !text.isEmpty ();
		if (!text.isEmpty ())
			Ui_.TextEditor_->setText (text);

		qint32 line = 0, index = 0;
		istr >> line >> index;
		Ui_.TextEditor_->setCursorPosition (line, index);
	}

	void EditorPage::SetText (const QString& text)
	{
		Ui_.TextEditor_->setText (text);
	}

	void EditorPage::SetLanguage (const QString& language)
	{
		if (!Extension2Lang_.values ().contains (language))
			return;

		Ui_.TextEditor_->setLexer (GetLexerByLanguage (language));
		emit languageChanged (language);
		DoctypeDetected_ = true;
	}

	void EditorPage::selectDoctype (QAction *action)
	{
		SetLanguage (action->text ());
	}

	void EditorPage::on_ActionNew__triggered ()
	{
		Filename_.clear ();

		Ui_.TextEditor_->setText (QString ());
		Modified_ = false;

		emit changeTabName (QString ("%1 - Popishu")
				.arg (tr ("Untitled")));

		DoctypeDetected_ = false;
	}

	void EditorPage::on_ActionOpen__triggered ()
	{
		QString filename = QFileDialog::getOpenFileName (this,
				tr ("Select file to open"));
		if (filename.isEmpty ())
			return;

		Open (filename);
	}

	void EditorPage::on_ActionSave__triggered ()
	{
		Save ();
	}

	void EditorPage::on_ActionSaveAs__triggered ()
	{
		QString fname = Filename_;
		Filename_.clear ();
		if (!Save ())
			Filename_ = fname;
	}

	void EditorPage::on_ActionWSInvisible__triggered ()
	{
		SetWhitespaceVisibility (QsciScintilla::WsInvisible);
	}

	void EditorPage::on_ActionWSVisible__triggered ()
	{
		SetWhitespaceVisibility (QsciScintilla::WsVisible);
	}

	void EditorPage::on_ActionWSVisibleAfterIndent__triggered ()
	{
		SetWhitespaceVisibility (QsciScintilla::WsVisibleAfterIndent);
	}

	void EditorPage::on_ActionShowLineNumbers__toggled (bool enable)
	{
		Ui_.TextEditor_->setMarginType (0, QsciScintilla::NumberMargin);
		Ui_.TextEditor_->setMarginWidth (0, "10000");
		Ui_.TextEditor_->setMarginLineNumbers (0, enable);
	}

	void EditorPage::on_ActionEnableFolding__toggled (bool enable)
	{
		Ui_.TextEditor_->setFolding (enable ?
				QsciScintilla::CircledTreeFoldStyle :
				QsciScintilla::NoFoldStyle);
	}

	void EditorPage::on_ActionWrapNone__triggered ()
	{
		Ui_.TextEditor_->setWrapMode (QsciScintilla::WrapNone);
	}

	void EditorPage::on_ActionWrapWords__triggered ()
	{
		Ui_.TextEditor_->setWrapMode (QsciScintilla::WrapWord);
	}

	void EditorPage::on_ActionWrapCharacters__triggered ()
	{
		Ui_.TextEditor_->setWrapMode (QsciScintilla::WrapCharacter);
	}

	void EditorPage::on_ActionReplace__triggered ()
	{
		ReplaceDialog dia { this };
		if (dia.exec () != QDialog::Accepted)
			return;

		QString before = dia.GetBefore ();
		QString after = dia.GetAfter ();
		Qt::CaseSensitivity cs = dia.GetCaseSensitivity ();
		switch (dia.GetScope ())
		{
		case ReplaceDialog::Scope::All:
		{
			QString text = Ui_.TextEditor_->text ();
			text.replace (before, after, cs);
			Ui_.TextEditor_->setText (text);
			break;
		}
		case ReplaceDialog::Scope::Selected:
		{
			QString text = Ui_.TextEditor_->selectedText ();
			text.replace (before, after, cs);

			int lineFrom = 0, indexFrom = 0,
					lineTo = 0, indexTo = 0;
			Ui_.TextEditor_->getSelection (&lineFrom, &indexFrom, &lineTo, &indexTo);
			Ui_.TextEditor_->removeSelectedText ();
			Ui_.TextEditor_->insertAt (text, lineFrom, indexFrom);
			break;
		}
		}
	}

	void EditorPage::on_TextEditor__textChanged ()
	{
		Modified_ = true;

		ScheduleTabRecoverSave ();
	}

	void EditorPage::on_TextEditor__cursorPositionChanged (int, int)
	{
		ScheduleTabRecoverSave ();
	}

	void EditorPage::on_Inject__released ()
	{
		if (!Save ())
			return;

		Entity e = Util::MakeEntity (QUrl::fromLocalFile (Filename_),
				QString (),
				FromUserInitiated | OnlyHandle,
				"x-leechcraft/script-wrap-request");
		e.Additional_ ["Object"] = QVariant::fromValue<QObject**> (&WrappedObject_);

		for (const auto action : DoctypeMenu_->actions ())
			if (action->isChecked ())
			{
				e.Additional_ ["Language"] = FixLanguage (action->text ());
				break;
			}

		// TODO rework to do this via some proper API
		if (!Proxy_->GetEntityManager ()->DelegateEntity (e) || !WrappedObject_)
		{
			qWarning () << Q_FUNC_INFO
					<< "script wrapping failed";
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Script wrapping failed."));
			return;
		}

		try
		{
			Proxy_->GetPluginsManager ()->InjectPlugin (WrappedObject_);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "script injection failed"
					<< e.what ();
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Script injection failed: %1")
						.arg (e.what ()));
			WrappedObject_->deleteLater ();

			return;
		}

		qDebug () << Q_FUNC_INFO
				<< "obtained"
				<< WrappedObject_;

		Ui_.Inject_->setEnabled (false);
		Ui_.Release_->setEnabled (true);
	}

	void EditorPage::on_Release__released ()
	{
		Ui_.Inject_->setEnabled (true);
		Ui_.Release_->setEnabled (false);

		try
		{
			Proxy_->GetPluginsManager ()->ReleasePlugin (WrappedObject_);
		}
		catch (const std::exception& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "script injection failed"
					<< e.what ();
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Script injection failed: %1")
						.arg (e.what ()));
			WrappedObject_->deleteLater ();
			return;
		}
		WrappedObject_ = 0;
	}

	void EditorPage::handleMonoFontChanged ()
	{
		auto lexer = Ui_.TextEditor_->lexer ();
		if (!lexer)
			return;

		auto font = XmlSettingsManager::Instance ().property ("MonoFont").value<QFont> ();
		lexer->setFont (font);
	}

	namespace
	{
		QsciScintilla::WrapVisualFlag FlagFromName (const QString& name)
		{
			if (name == "text")
				return QsciScintilla::WrapFlagByText;
			else if (name == "border")
				return QsciScintilla::WrapFlagByBorder;
			else
				return QsciScintilla::WrapFlagNone;
		}
	}

	void EditorPage::handleVisualWrapFlags ()
	{
		auto& xsm = XmlSettingsManager::Instance ();
		auto eflag = FlagFromName (xsm.property ("EndLineFlag").toString ());
		auto sflag = FlagFromName (xsm.property ("StartLineFlag").toString ());
		int indent = xsm.property ("WrappedIndent").toInt ();
		Ui_.TextEditor_->setWrapVisualFlags (eflag, sflag, indent);
	}

	void EditorPage::handleOtherPrefs ()
	{
		auto& xsm = XmlSettingsManager::Instance ();
		Ui_.TextEditor_->setTabWidth (xsm.property ("TabWidth").toInt ());
		Ui_.TextEditor_->setTabWidth (xsm.property ("IndentationWidth").toInt ());
	}

	void EditorPage::checkInterpreters (QString language)
	{
		Entity e = Util::MakeEntity (QUrl::fromLocalFile (Filename_),
				QString (),
				FromUserInitiated,
				"x-leechcraft/script-wrap-request");
		QObject *object = nullptr;
		e.Additional_ ["Object"] = QVariant::fromValue<QObject**> (&object);
		e.Additional_ ["Language"] = FixLanguage (language);

		const auto ch = Proxy_->GetEntityManager ()->CouldHandle (e);
		ShowConsole (ch);
	}

	void EditorPage::checkProperDoctypeAction (const QString& language)
	{
		for (QAction *act : DoctypeMenu_->actions ())
		{
			act->blockSignals (true);
			act->setChecked (act->text () == language);
			act->blockSignals (false);
		}
	}

	void EditorPage::tabRecoverSave ()
	{
		TabRecoverSaveScheduled_ = false;
		emit tabRecoverDataChanged ();
	}

	void EditorPage::SetWhitespaceVisibility (QsciScintilla::WhitespaceVisibility wv)
	{
		Ui_.TextEditor_->setWhitespaceVisibility (wv);
	}

	bool EditorPage::Save ()
	{
		if (Filename_.isEmpty ())
		{
			Filename_ = QFileDialog::getSaveFileName (this,
					tr ("Select file to save"));
			if (Filename_.isEmpty ())
				return false;

			emit changeTabName (QString ("%1 - Popishu")
					.arg (Filename_));
		}

		QFile file (Filename_);
		if (!file.open (QIODevice::WriteOnly))
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Could not open file %1 for writing.")
						.arg (Filename_));
			return false;
		}

		file.write (Ui_.TextEditor_->text ().toUtf8 ());

		if (!DoctypeDetected_)
		{
			const auto& language = GetLanguage (Filename_);
			const auto lexer = GetLexerByLanguage (language);
			Ui_.TextEditor_->setLexer (lexer);
			emit languageChanged (language);
			DoctypeDetected_ = lexer;
		}

		Modified_ = false;

		TemporaryDocument_ = false;

		return true;
	}

	void EditorPage::Open (const QString& filename)
	{
		QFile file (filename);
		if (!file.open (QIODevice::ReadOnly))
		{
			QMessageBox::critical (this,
					"LeechCraft",
					tr ("Could not open file %1 for reading.")
						.arg (filename));
			return;
		}

		Filename_ = filename;
		Ui_.TextEditor_->read (&file);

		const auto& language = GetLanguage (Filename_);
		const auto lexer = GetLexerByLanguage (language);
		Ui_.TextEditor_->setLexer (lexer);
		DoctypeDetected_ = lexer;
		emit languageChanged (language);

		Modified_ = false;

		emit changeTabName (QString ("%1 - Popishu")
				.arg (Filename_));
		QStringList path ("Popishu");
		path += Filename_.split ('/', Qt::SkipEmptyParts);
		setProperty ("WidgetLogicalPath", path);

		PrependRecentFile (filename);
	}

	QsciLexer* EditorPage::GetLexerByLanguage (const QString& lang) const
	{
		QsciLexer *result = nullptr;
		if (lang == "Bash")
			result = new QsciLexerBash (Ui_.TextEditor_);
		else if (lang == "CMake")
			result = new QsciLexerCMake (Ui_.TextEditor_);
		else if (lang == "C++")
			result = new QsciLexerCPP (Ui_.TextEditor_);
		else if (lang == "CSS")
			result = new QsciLexerCSS (Ui_.TextEditor_);
		else if (lang == "Diff")
			result = new QsciLexerDiff (Ui_.TextEditor_);
		else if (lang == "HTML")
			result = new QsciLexerHTML (Ui_.TextEditor_);
		else if (lang == "JavaScript")
			result = new QsciLexerJavaScript (Ui_.TextEditor_);
		else if (lang == "MatLab")
			result = new QsciLexerMatlab (Ui_.TextEditor_);
		else if (lang == "Octave")
			result = new QsciLexerOctave (Ui_.TextEditor_);
		else if (lang == "Python")
			result = new QsciLexerPython (Ui_.TextEditor_);
		else if (lang == "Ruby")
			result = new QsciLexerRuby (Ui_.TextEditor_);
		else if (lang == "SQL")
			result = new QsciLexerSQL (Ui_.TextEditor_);
		else if (lang == "TeX")
			result = new QsciLexerTeX (Ui_.TextEditor_);
		else if (lang == "XML")
			result = new QsciLexerXML (Ui_.TextEditor_);

		if (result)
			result->setFont (XmlSettingsManager::Instance ().property ("MonoFont").value<QFont> ());

		return result;
	}

	QString EditorPage::GetLanguage (const QString& name) const
	{
		return Extension2Lang_ [QFileInfo (name).suffix ()];
	}

	QString EditorPage::FixLanguage (const QString& language) const
	{
		if (language.toLower () == "javascript")
			return "qtscript";
		else
			return language;
	}

	void EditorPage::ShowConsole (bool show)
	{
		Ui_.ConsoleBox_->setVisible (show);
		Ui_.Splitter_->refresh ();
	}

	void EditorPage::GroupActions (const QList<QAction*>& actions)
	{
		if (actions.isEmpty ())
			return;

		auto group = new QActionGroup (this);
		for (const auto action : actions)
			group->addAction (action);
	}

	void EditorPage::RestoreRecentFiles ()
	{
		auto recent = XmlSettingsManager::Instance ().property ("RecentlyOpenedFiles").toStringList ();
		int num = XmlSettingsManager::Instance ().property ("NumRecentlyOpened").toInt ();
		while (recent.size () > num)
			recent.removeAt (num);

		std::reverse (recent.begin (), recent.end ());
		for (const auto& filePath : recent)
			PrependRecentFile (filePath, false);
	}

	void EditorPage::SetupDefPairs ()
	{
#define DEFPAIR(l,e) Extension2Lang_ [#e] = #l;
		DEFPAIR (Bash, sh);
		DEFPAIR (CMake, cmake);
		DEFPAIR (C++, cpp);
		DEFPAIR (C++, h);
		DEFPAIR (C++, cxx);
		DEFPAIR (C++, hxx);
		DEFPAIR (CSS, css);
		DEFPAIR (Diff, diff);
		DEFPAIR (Diff, patch);
		DEFPAIR (HTML, htm);
		DEFPAIR (HTML, html);
		DEFPAIR (HTML, xhtml);
		DEFPAIR (JavaScript, es);
		DEFPAIR (JavaScript, js);
		DEFPAIR (JavaScript, qs);
		DEFPAIR (MatLab, mat);
		DEFPAIR (Octave, m);
		DEFPAIR (Python, py);
		DEFPAIR (Ruby, rb);
		DEFPAIR (SQL, sql);
		DEFPAIR (TeX, tex);
		DEFPAIR (XML, xml);
#undef DEFPAIR
	}

	void EditorPage::ScheduleTabRecoverSave ()
	{
		if (TabRecoverSaveScheduled_)
			return;

		TabRecoverSaveScheduled_ = true;
		QTimer::singleShot (5000,
				this,
				SLOT (tabRecoverSave ()));
	}

	void EditorPage::PrependRecentFile (const QString& filePath, bool save)
	{
		auto action = new QAction (filePath, this);
		action->setData (filePath);
		connect (action,
				&QAction::triggered,
				[this, filePath]
				{
					if (!QFile::exists (filePath))
						Proxy_->GetEntityManager ()->HandleEntity (Util::MakeNotification (tr ("File not found"),
									tr ("The requested file doesn't exist anymore."),
									Priority::Warning));
					else
						Open (filePath);
				});

		QList<QAction*> currentActions = RecentFilesMenu_->actions ();
		if (currentActions.isEmpty ())
			RecentFilesMenu_->addAction (action);
		else
		{
			for (const auto otherAct : currentActions)
				if (otherAct->data ().toString () == filePath)
				{
					currentActions.removeAll (otherAct);
					delete otherAct;
				}

			RecentFilesMenu_->insertAction (currentActions.at (0), action);
			int num = XmlSettingsManager::Instance ().property ("NumRecentlyOpened").toInt ();
			while (currentActions.size () + 1 > num)
				delete currentActions.takeLast ();
		}

		if (save)
		{
			currentActions.prepend (action);
			const auto& recent = Util::Map (currentActions, [] (QAction *act) { return act->data ().toString (); });
			XmlSettingsManager::Instance ().setProperty ("RecentlyOpenedFiles", recent);
		}
	}

	void EditorPage::SetTemporaryDocument (bool tempDocument)
	{
		TemporaryDocument_ = tempDocument;
	}

	QsciScintilla* EditorPage::GetTextEditor () const
	{
		return Ui_.TextEditor_;
	}
}
}
