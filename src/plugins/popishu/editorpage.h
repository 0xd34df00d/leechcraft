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

#ifndef PLUGINS_POPISHU_EDITORPAGE_H
#define PLUGINS_POPISHU_EDITORPAGE_H
#include <QWidget>
#include <QHash>
#include <interfaces/imultitabs.h>
#include <interfaces/structures.h>
#include "ui_editorpage.h"
#include <boost/graph/graph_concepts.hpp>

class QMenu;

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Popishu
		{
			class EditorPage : public QWidget
							 , public IMultiTabsWidget
			{
				Q_OBJECT
				Q_INTERFACES (IMultiTabsWidget)

				Ui::EditorPage Ui_;

				static QObject* S_MultiTabsParent_;

				QToolBar *Toolbar_;
				QMenu *DoctypeMenu_;
				QMenu *RecentFilesMenu_;
				QString Filename_;
				bool Modified_;
				QMap<QString, QList<QAction*> > WindowMenus_;
				QHash<QString, QString> Extension2Lang_;

				QtMsgHandler DefaultMsgHandler_;
				QObject *WrappedObject_;
				bool PoshukiPageSourceCode_;
			public:
				static void SetParentMultiTabs (QObject*);

				EditorPage (QWidget* = 0);
				virtual ~EditorPage ();

				void Remove ();
				QToolBar* GetToolBar () const;
				void NewTabRequested ();
				QObject* ParentMultiTabs () const;
				QList<QAction*> GetTabBarContextMenuActions () const;
				QMap<QString, QList<QAction*> > GetWindowMenus () const;

				void SetText (const QString&);
				void SetLanguage (const QString&);
				
				void SetPoshukiPageSourceCode (bool);
			private slots:
				void selectDoctype (QAction*);
				void on_ActionNew__triggered ();
				void on_ActionOpen__triggered ();
				void on_ActionSave__triggered ();
				void on_ActionSaveAs__triggered ();
				void on_ActionWSInvisible__triggered ();
				void on_ActionWSVisible__triggered ();
				void on_ActionWSVisibleAfterIndent__triggered ();
				void on_ActionShowLineNumbers__toggled (bool);
				void on_ActionEnableFolding__toggled (bool);
				void on_ActionWrapNone__triggered ();
				void on_ActionWrapWords__triggered ();
				void on_ActionWrapCharacters__triggered ();
				void on_ActionReplace__triggered ();
				void on_TextEditor__textChanged ();
				void on_Inject__released ();
				void on_Release__released ();

				void handleMonoFontChanged ();
				void handleVisualWrapFlags ();
				void handleOtherPrefs ();

				void checkInterpreters (QString language);
				void checkProperDoctypeAction (const QString& language);

				void handleRecentFileOpen ();
			private:
				void SetWhitespaceVisibility (QsciScintilla::WhitespaceVisibility);
				bool Save ();
				void Open (const QString&);
				QsciLexer* GetLexerByLanguage (const QString&) const;
				QString GetLanguage (const QString& filename) const;
				QString FixLanguage (const QString&) const;
				void ShowConsole (bool);
				void GroupActions (const QList<QAction*>&);
				void RestoreRecentFiles ();
				void PrependRecentFile (const QString&, bool = true);
			signals:
				void removeTab (QWidget*);
				void changeTabName (QWidget*, const QString&);
				void changeTabIcon (QWidget*, const QIcon&);
				void changeTooltip (QWidget*, QWidget*);
				void statusBarChanged (QWidget*, const QString&);
				void couldHandle (const LeechCraft::Entity&, bool*);
				void delegateEntity (const LeechCraft::Entity&,
						int*, QObject**);
				void gotEntity (const LeechCraft::Entity&);

				void languageChanged (const QString& language);
			};
		};
	};
};

#endif
