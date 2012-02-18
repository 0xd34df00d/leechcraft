/**********************************************************************
1 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "editorpage.h"
#include <iostream>
#include <algorithm>
#include <QToolBar>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QMenu>
#include <QFileInfo>
#include <QUrl>
#include <Qsci/qscilexerbash.h>
#include <Qsci/qscilexercmake.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerdiff.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexerjavascript.h>
#if QSCINTILLA_VERSION >= 0x020501
#include <Qsci/qscilexermatlab.h>
#include <Qsci/qscilexeroctave.h>
#endif
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerruby.h>
#include <Qsci/qscilexersql.h>
#include <Qsci/qscilexertex.h>
#include <Qsci/qscilexerxml.h>
#include <util/util.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "replacedialog.h"

Q_DECLARE_METATYPE (QObject**);

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Popishu
		{
			QObject *EditorPage::S_MultiTabsParent_ = 0;

			EditorPage::EditorPage (QWidget *parent)
			: QWidget (parent)
			, Toolbar_ (new QToolBar)
			, Modified_ (false)
			, DefaultMsgHandler_ (0)
			, WrappedObject_ (0)
			, TemporaryDocument_ (false)
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
#if QSCINTILLA_VERSION >= 0x020501
				DEFPAIR (MatLab, mat);
				DEFPAIR (Octave, m);
#endif
				DEFPAIR (Python, py);
				DEFPAIR (Ruby, rb);
				DEFPAIR (SQL, sql);
				DEFPAIR (TeX, tex);
				DEFPAIR (XML, xml);
#undef DEFPAIR
				Ui_.setupUi (this);

				Toolbar_->addAction (Ui_.ActionNew_);
				Toolbar_->addAction (Ui_.ActionOpen_);
				Toolbar_->addAction (Ui_.ActionSave_);
				Toolbar_->addAction (Ui_.ActionSaveAs_);

				Ui_.TextEditor_->setAutoIndent (true);
				Ui_.TextEditor_->setUtf8 (true);

				DoctypeMenu_ = new QMenu (tr ("Document type"));
				DoctypeMenu_->addAction ("Bash")->setCheckable (true);
				DoctypeMenu_->addAction ("CMake")->setCheckable (true);
				DoctypeMenu_->addAction ("C++")->setCheckable (true);
				DoctypeMenu_->addAction ("CSS")->setCheckable (true);
				DoctypeMenu_->addAction ("Diff")->setCheckable (true);
				DoctypeMenu_->addAction ("HTML")->setCheckable (true);
				DoctypeMenu_->addAction ("JavaScript")->setCheckable (true);
#if QSCINTILLA_VERSION >= 0x020501
				DoctypeMenu_->addAction ("MatLab")->setCheckable (true);
				DoctypeMenu_->addAction ("Octave")->setCheckable (true);
#endif
				DoctypeMenu_->addAction ("Python")->setCheckable (true);
				DoctypeMenu_->addAction ("Ruby")->setCheckable (true);
				DoctypeMenu_->addAction ("SQL")->setCheckable (true);
				DoctypeMenu_->addAction ("TeX")->setCheckable (true);
				DoctypeMenu_->addAction ("XML")->setCheckable (true);
				connect (DoctypeMenu_,
						SIGNAL (triggered (QAction*)),
						this,
						SLOT (selectDoctype (QAction*)));
				connect (this,
						SIGNAL (languageChanged (const QString&)),
						this,
						SLOT (checkProperDoctypeAction (const QString&)));
				connect (this,
						SIGNAL (languageChanged (const QString&)),
						this,
						SLOT (checkInterpreters (const QString&)));

				RecentFilesMenu_ = new QMenu (tr ("Recent files"));
				RestoreRecentFiles ();

				QString editor = "view";
				WindowMenus_ [editor] << Ui_.ActionEnableFolding_;
				WindowMenus_ [editor] << Ui_.ActionAutoIndent_;
				WindowMenus_ [editor] << Ui_.ActionShowLineNumbers_;

				WindowMenus_ [editor] << Util::CreateSeparator (this);

				WindowMenus_ [editor] << DoctypeMenu_->menuAction ();

				QMenu *wsVis = new QMenu (tr ("Whitespace visibility"));
				wsVis->addAction (Ui_.ActionWSInvisible_);
				wsVis->addAction (Ui_.ActionWSVisible_);
				wsVis->addAction (Ui_.ActionWSVisibleAfterIndent_);
				WindowMenus_ [editor] << wsVis->menuAction ();
				GroupActions (wsVis->actions ());

				QMenu *wrapMode = new QMenu (tr ("Wrapping mode"));
				wrapMode->addAction (Ui_.ActionWrapNone_);
				wrapMode->addAction (Ui_.ActionWrapWords_);
				wrapMode->addAction (Ui_.ActionWrapCharacters_);
				WindowMenus_ [editor] << wrapMode->menuAction ();
				GroupActions (wrapMode->actions ());

				WindowMenus_ [editor] << Util::CreateSeparator (this);

				WindowMenus_ [editor] << Ui_.ActionShowEOL_;
				WindowMenus_ [editor] << Ui_.ActionShowCaretLine_;
				WindowMenus_ [editor] << Util::CreateSeparator (this);

				QString edit = tr ("Edit");
				WindowMenus_ [edit] << Ui_.ActionReplace_;

				QString file = tr ("File");
				WindowMenus_ [file] << Ui_.ActionNew_;
				WindowMenus_ [file] << Ui_.ActionOpen_;
				WindowMenus_ [file] << RecentFilesMenu_->menuAction ();
				WindowMenus_ [file] << Ui_.ActionSave_;
				WindowMenus_ [file] << Ui_.ActionSaveAs_;

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

				XmlSettingsManager::Instance ()->
						RegisterObject ("MonoFont", this, "handleMonoFontChanged");

				QList<QByteArray> visualWrapPrefs;
				visualWrapPrefs << "EndLineFlag"
						<< "StartLineFlag"
						<< "WrappedIndent";
				XmlSettingsManager::Instance ()->
						RegisterObject (visualWrapPrefs, this, "handleVisualWrapFlags");
				handleVisualWrapFlags ();

				QList<QByteArray> otherPrefs;
				otherPrefs << "TabWidget"
						<< "IdentationWidth";
				XmlSettingsManager::Instance ()->
						RegisterObject (otherPrefs, this, "handleOtherPrefs");
				handleOtherPrefs ();

				ShowConsole (false);

				Ui_.ActionWrapWords_->trigger ();
				Ui_.ActionShowLineNumbers_->trigger ();
			}

			EditorPage::~EditorPage ()
			{
				if (DefaultMsgHandler_)
					qInstallMsgHandler (DefaultMsgHandler_);
				if (WrappedObject_)
					Core::Instance ().GetProxy ()->
							GetPluginsManager ()->ReleasePlugin (WrappedObject_);
			}

			void EditorPage::SetParentMultiTabs (QObject *parent)
			{
				S_MultiTabsParent_ = parent;
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

				emit removeTab (this);
				deleteLater ();
			}

			QToolBar* EditorPage::GetToolBar () const
			{
				return Toolbar_;
			}

			QObject* EditorPage::ParentMultiTabs ()
			{
				return S_MultiTabsParent_;
			}

			QList<QAction*> EditorPage::GetTabBarContextMenuActions () const
			{
				return QList<QAction*> ();
			}

			QMap<QString, QList<QAction*> > EditorPage::GetWindowMenus () const
			{
				return WindowMenus_;
			}

			TabClassInfo EditorPage::GetTabClassInfo () const
			{
				return Core::Instance ().GetTabClass ();
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
			}

			void EditorPage::selectDoctype (QAction *action)
			{
				QString name = action->text ();
				Ui_.TextEditor_->setLexer (GetLexerByLanguage (name));
				emit languageChanged (name);
			}

			void EditorPage::on_ActionNew__triggered ()
			{
				Filename_.clear ();

				Ui_.TextEditor_->setText (QString ());
				Modified_ = false;

				emit changeTabName (this, QString ("%1 - Popishu")
						.arg (tr ("Untitled")));
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
				std::auto_ptr<ReplaceDialog> dia (new ReplaceDialog (this));
				if (dia->exec () != QDialog::Accepted)
					return;

				QString before = dia->GetBefore ();
				QString after = dia->GetAfter ();
				Qt::CaseSensitivity cs = dia->GetCaseSensitivity ();
				switch (dia->GetScope ())
				{
				case ReplaceDialog::SAll:
				{
					QString text = Ui_.TextEditor_->text ();
					text.replace (before, after, cs);
					Ui_.TextEditor_->setText (text);
				}
				case ReplaceDialog::SSelected:
				{
					QString text = Ui_.TextEditor_->selectedText ();
					text.replace (before, after, cs);

					int lineFrom = 0, indexFrom = 0,
							lineTo = 0, indexTo = 0;
					Ui_.TextEditor_->getSelection (&lineFrom, &indexFrom, &lineTo, &indexTo);
					Ui_.TextEditor_->removeSelectedText ();
					Ui_.TextEditor_->insertAt (text, lineFrom, indexFrom);
				}
				}
			}

			void EditorPage::on_TextEditor__textChanged ()
			{
				Modified_ = true;
			}

			static QPlainTextEdit *S_TextEdit_ = 0;

			void output (QtMsgType type, const char *msg)
			{
				QString line;
				switch (type)
				{
				case QtDebugMsg:
					line = "Debug: ";
					break;
				case QtWarningMsg:
					line = "Warning: ";
					break;
				case QtCriticalMsg:
					line = "Critical: ";
					break;
				case QtFatalMsg:
					std::cerr << "Fatal: " << msg;
					abort ();
				}

				line += msg;
				if (S_TextEdit_)
					S_TextEdit_->appendPlainText (line);
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

				Q_FOREACH (QAction *action, DoctypeMenu_->actions ())
					if (action->isChecked ())
					{
						e.Additional_ ["Language"] = FixLanguage (action->text ());
						break;
					}

				S_TextEdit_ = Ui_.Console_;
				DefaultMsgHandler_ = qInstallMsgHandler (output);

				emit delegateEntity (e, 0, 0);

				if (!WrappedObject_)
				{
					qWarning () << Q_FUNC_INFO
							<< "script wrapping failed";
					QMessageBox::critical (this,
							"LeechCraft",
							tr ("Script wrapping failed."));

					qInstallMsgHandler (DefaultMsgHandler_);
					S_TextEdit_ = 0;
					DefaultMsgHandler_ = 0;

					return;
				}

				try
				{
					Core::Instance ().GetProxy ()->
							GetPluginsManager ()->InjectPlugin (WrappedObject_);
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

					qInstallMsgHandler (DefaultMsgHandler_);
					S_TextEdit_ = 0;
					DefaultMsgHandler_ = 0;

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
					Core::Instance ().GetProxy ()->
							GetPluginsManager ()->ReleasePlugin (WrappedObject_);
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

				if (DefaultMsgHandler_)
				{
					S_TextEdit_ = 0;
					qInstallMsgHandler (DefaultMsgHandler_);
					DefaultMsgHandler_ = 0;
				}
			}

			void EditorPage::handleMonoFontChanged ()
			{
				QsciLexer *lexer = Ui_.TextEditor_->lexer ();
				if (!lexer)
					return;

				QFont font = XmlSettingsManager::Instance ()->
						property ("MonoFont").value<QFont> ();
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
				QsciScintilla::WrapVisualFlag eflag =
						FlagFromName (XmlSettingsManager::Instance ()->
								property ("EndLineFlag").toString ());
				QsciScintilla::WrapVisualFlag sflag =
						FlagFromName (XmlSettingsManager::Instance ()->
								property ("StartLineFlag").toString ());;
				int indent = XmlSettingsManager::Instance ()->
						property ("WrappedIndent").toInt ();
				Ui_.TextEditor_->setWrapVisualFlags (eflag, sflag, indent);
			}

			void EditorPage::handleOtherPrefs ()
			{
				Ui_.TextEditor_->setTabWidth (XmlSettingsManager::Instance ()->
						property ("TabWidth").toInt ());
				Ui_.TextEditor_->setTabWidth (XmlSettingsManager::Instance ()->
						property ("IndentationWidth").toInt ());
			}

			void EditorPage::checkInterpreters (QString language)
			{
				Entity e = Util::MakeEntity (QUrl::fromLocalFile (Filename_),
						QString (),
						FromUserInitiated,
						"x-leechcraft/script-wrap-request");
				QObject *object = 0;
				e.Additional_ ["Object"] = QVariant::fromValue<QObject**> (&object);
				e.Additional_ ["Language"] = FixLanguage (language);

				bool ch = false;
				emit couldHandle (e, &ch);
				ShowConsole (ch);
			}

			void EditorPage::checkProperDoctypeAction (const QString& language)
			{
				Q_FOREACH (QAction *act, DoctypeMenu_->actions ())
				{
					act->blockSignals (true);
					act->setChecked (act->text () == language);
					act->blockSignals (false);
				}
			}

			void EditorPage::handleRecentFileOpen ()
			{
				QAction *sAct = qobject_cast<QAction*> (sender ());
				if (!sAct)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender is not a QAction"
							<< sender ();
					return;
				}

				QString file = sAct->data ().toString ();
				if (!QFile::exists (file))
				{
					emit gotEntity (Util::MakeNotification (tr ("File not found"),
							tr ("The requested file doesn't exist anymore."),
							PWarning_));
					return;
				}

				Open (file);
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

					emit changeTabName (this, QString ("%1 - Popishu")
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

				Ui_.TextEditor_->setLexer (GetLexerByLanguage (GetLanguage (Filename_)));
				emit languageChanged (GetLanguage (Filename_));

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
				Ui_.TextEditor_->setText (QString::fromUtf8 (file
							.readAll ().constData ()));

				Ui_.TextEditor_->setLexer (GetLexerByLanguage (GetLanguage (Filename_)));
				emit languageChanged (GetLanguage (Filename_));

				Modified_ = false;

				emit changeTabName (this, QString ("%1 - Popishu")
						.arg (Filename_));
				QStringList path ("Popishu");
				path += Filename_.split ('/', QString::SkipEmptyParts);
				setProperty ("WidgetLogicalPath", path);

				PrependRecentFile (filename);
			}

			QsciLexer* EditorPage::GetLexerByLanguage (const QString& lang) const
			{
				QsciLexer *result = 0;
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
#if QSCINTILLA_VERSION >= 0x020501
				else if (lang == "MatLab")
					result = new QsciLexerMatlab (Ui_.TextEditor_);
				else if (lang == "Octave")
					result = new QsciLexerOctave (Ui_.TextEditor_);
#endif
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
					result->setFont (XmlSettingsManager::Instance ()->
							property ("MonoFont").value<QFont> ());

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
				if (!actions.size ())
					return;

				QActionGroup *group = new QActionGroup (this);
				Q_FOREACH (QAction *action, actions)
					group->addAction (action);
			}

			void EditorPage::RestoreRecentFiles ()
			{
				QStringList recent = XmlSettingsManager::Instance ()->
						property ("RecentlyOpenedFiles").toStringList ();
				int num = XmlSettingsManager::Instance ()->
						property ("NumRecentlyOpened").toInt ();
				while (recent.size () > num)
					recent.removeAt (num);

				std::reverse (recent.begin (), recent.end ());
				Q_FOREACH (const QString& filePath, recent)
					PrependRecentFile (filePath, false);
			}

			void EditorPage::PrependRecentFile (const QString& filePath, bool save)
			{
				QAction *action = new QAction (filePath, this);
				action->setData (filePath);
				connect (action,
						SIGNAL (triggered ()),
						this,
						SLOT (handleRecentFileOpen ()));

				QList<QAction*> currentActions = RecentFilesMenu_->actions ();
				if (!currentActions.size ())
					RecentFilesMenu_->addAction (action);
				else
				{
					Q_FOREACH (QAction *action, currentActions)
						if (action->data ().toString () == filePath)
						{
							currentActions.removeAll (action);
							delete action;
						}

					RecentFilesMenu_->insertAction (currentActions.at (0), action);
					int num = XmlSettingsManager::Instance ()->
							property ("NumRecentlyOpened").toInt ();
					while (currentActions.size () + 1 > num)
						delete currentActions.takeLast ();
				}

				if (save)
				{
					QStringList recent;
					currentActions.prepend (action);
					Q_FOREACH (QAction *action, currentActions)
						recent << action->data ().toString ();
					XmlSettingsManager::Instance ()->
							setProperty ("RecentlyOpenedFiles", recent);
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
		};
	};
};
