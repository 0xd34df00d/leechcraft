/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <QHash>
#include <interfaces/ihavetabs.h>
#include <interfaces/structures.h>
#include <interfaces/ihaverecoverabletabs.h>
#include <interfaces/core/icoreproxy.h>
#include "ui_editorpage.h"

class QMenu;

namespace LC
{
namespace Popishu
{
	class EditorPage : public QWidget
					 , public ITabWidget
					 , public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		Ui::EditorPage Ui_;

		const TabClassInfo TC_;
		QObject * const ParentPlugin_;
		const ICoreProxy_ptr Proxy_;

		std::unique_ptr<QToolBar> Toolbar_;
		QMenu *DoctypeMenu_;
		QMenu *RecentFilesMenu_;
		QString Filename_;
		bool Modified_ = false;
		QHash<QString, QString> Extension2Lang_;

		bool DoctypeDetected_ = false;

		QObject *WrappedObject_ = nullptr;
		bool TemporaryDocument_ = false;

		bool TabRecoverSaveScheduled_ = false;
	public:
		EditorPage (const ICoreProxy_ptr& proxy, const TabClassInfo&, QObject*);
		~EditorPage ();

		void Remove () override;
		QToolBar* GetToolBar () const override;
		QObject* ParentMultiTabs () override;
		TabClassInfo GetTabClassInfo () const override;

		QByteArray GetTabRecoverData () const override;
		QIcon GetTabRecoverIcon () const override;
		QString GetTabRecoverName () const override;

		void RestoreState (QDataStream&);

		void SetText (const QString&);
		void SetLanguage (const QString&);

		void SetTemporaryDocument (bool);
		QsciScintilla* GetTextEditor () const;
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
		void on_TextEditor__cursorPositionChanged (int, int);

		void on_Inject__released ();
		void on_Release__released ();

		void handleMonoFontChanged ();
		void handleVisualWrapFlags ();
		void handleOtherPrefs ();

		void checkInterpreters (QString language);
		void checkProperDoctypeAction (const QString& language);

		void tabRecoverSave ();
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
		void SetupDefPairs ();

		void ScheduleTabRecoverSave ();

		void PrependRecentFile (const QString&, bool = true);
	signals:
		void removeTab () override;
		void changeTabName (const QString&) override;

		void languageChanged (const QString& language);

		void tabRecoverDataChanged () override;
	};
}
}
