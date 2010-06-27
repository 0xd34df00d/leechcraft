/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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
#include <QToolBar>
#include <QFileDialog>
#include <QFile>
#include <QMessageBox>
#include <QMenu>
#include <QFileInfo>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexerjavascript.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexerxml.h>
#include "core.h"

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
			{
#define DEFPAIR(l,e) Extension2Lang_ [#e] = #l;
				DEFPAIR (C++, cpp);
				DEFPAIR (C++, h);
				DEFPAIR (C++, cxx);
				DEFPAIR (C++, hxx);
				DEFPAIR (CSS, css);
				DEFPAIR (HTML, htm);
				DEFPAIR (HTML, html);
				DEFPAIR (HTML, xhtml);
				DEFPAIR (JavaScript, es);
				DEFPAIR (JavaScript, js);
				DEFPAIR (JavaScript, qs);
				DEFPAIR (Python, py);
				DEFPAIR (XML, xml);
#undef DEFPAIR
				Ui_.setupUi (this);

				Toolbar_->addAction (Ui_.ActionNew_);
				Toolbar_->addAction (Ui_.ActionOpen_);
				Toolbar_->addAction (Ui_.ActionSave_);
				Toolbar_->addAction (Ui_.ActionSaveAs_);

				Ui_.TextEditor_->setAutoIndent (true);
				Ui_.TextEditor_->setUtf8 (true);

				DoctypeMenu_ = new QMenu ("Document type");
				DoctypeMenu_->addAction ("C++")->setCheckable (true);
				DoctypeMenu_->addAction ("CSS")->setCheckable (true);
				DoctypeMenu_->addAction ("HTML")->setCheckable (true);
				DoctypeMenu_->addAction ("JavaScript")->setCheckable (true);
				DoctypeMenu_->addAction ("Python")->setCheckable (true);
				DoctypeMenu_->addAction ("XML")->setCheckable (true);
				connect (DoctypeMenu_,
						SIGNAL (triggered (QAction*)),
						this,
						SLOT (selectDoctype (QAction*)));
				connect (this,
						SIGNAL (languageChanged (const QString&)),
						this,
						SLOT (checkProperDoctypeAction (const QString&)));
				WindowMenus_ ["tools"] << DoctypeMenu_->menuAction ();
			}

			void EditorPage::SetParentMultiTabs (QObject *parent)
			{
				S_MultiTabsParent_ = parent;
			}

			void EditorPage::Remove ()
			{
				if (Modified_)
				{
					QString name = QFileInfo (Filename_).fileName ();
					if (name.isEmpty ())
						name = tr ("Untitled");

					QMessageBox::StandardButton res =
							QMessageBox::question (this,
									tr ("LeechCraft"),
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

			void EditorPage::NewTabRequested ()
			{
				Core::Instance ().NewTabRequested ();
			}

			QObject* EditorPage::ParentMultiTabs () const
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
				Ui_.TextEditor_->setText (file.readAll ());

				Ui_.TextEditor_->setLexer (GetLexerByLanguage (GetLanguage (Filename_)));
				emit languageChanged (GetLanguage (Filename_));

				Modified_ = false;

				emit changeTabName (this, QString ("%1 - Popishu")
						.arg (Filename_));
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

			void EditorPage::on_TextEditor__textChanged ()
			{
				Modified_ = true;
			}

			void EditorPage::checkInterpreters (const QString& language)
			{
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

				return true;
			}

			QsciLexer* EditorPage::GetLexerByLanguage (const QString& lang) const
			{
				if (lang == "C++")
					return new QsciLexerCPP (Ui_.TextEditor_);
				else if (lang == "CSS")
					return new QsciLexerCSS (Ui_.TextEditor_);
				else if (lang == "HTML")
					return new QsciLexerHTML (Ui_.TextEditor_);
				else if (lang == "JavaScript")
					return new QsciLexerJavaScript (Ui_.TextEditor_);
				else if (lang == "Python")
					return new QsciLexerPython (Ui_.TextEditor_);
				else if (lang == "XML")
					return new QsciLexerXML (Ui_.TextEditor_);
				else
					return 0;
			}

			QString EditorPage::GetLanguage (const QString& name) const
			{
				return Extension2Lang_ [QFileInfo (name).suffix ()];
			}
		};
	};
};
