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
#include <interfaces/core/icoreproxy.h>
#include <interfaces/itexteditor.h>
#include <interfaces/iadvancedhtmleditor.h>
#include <interfaces/iwkfontssettable.h>
#include "ui_richeditorwidget.h"

class QToolBar;

namespace LC
{
namespace LHTR
{
	class RichEditorWidget final : public QWidget
								 , public IEditorWidget
								 , public IAdvancedHTMLEditor
								 , public IWkFontsSettable
	{
		Q_OBJECT
		Q_INTERFACES (IEditorWidget IAdvancedHTMLEditor IWkFontsSettable)

		ICoreProxy_ptr Proxy_;
		Ui::RichEditorWidget Ui_;

		QToolBar * const ViewBar_;

		QHash<QWebPage::WebAction, QAction*> WebAction2Action_;
		QHash<QString, QHash<QString, QAction*>> Cmd2Action_;

		CustomTags_t CustomTags_;

		bool HTMLDirty_ = false;

		QAction *FindAction_;
		QAction *ReplaceAction_;

		QAction *Bold_;
		QAction *Italic_;
		QAction *Underline_;

		QAction *InsertLink_;
		QAction *InsertImage_;

		QAction *ToggleView_;
	public:
		explicit RichEditorWidget (ICoreProxy_ptr, QWidget* = nullptr);

		QString GetContents (ContentType) const override;
		void SetContents (QString, ContentType) override;
		void AppendAction (QAction*) override;
		void AppendSeparator () override;
		void RemoveAction (QAction*) override;
		QAction* GetEditorAction (EditorAction) override;
		void SetBackgroundColor (const QColor&, ContentType) override;
		QWidget* GetWidget () override;
		QObject* GetQObject () override;

		void InsertHTML (const QString&) override;
		void SetCustomTags (const QList<CustomTag>&) override;
		QAction* AddInlineTagInserter (const QString& tagName, const QVariantMap& params) override;
		void ExecJS (const QString&) override;

		void SetFontFamily (FontFamily, const QFont&) override;
		void SetFontSize (FontSize, int) override;

		bool eventFilter (QObject*, QEvent*) override;
	private:
		void InternalSetBgColor (const QColor&, ContentType);

		void SetupImageMenu ();
		void SetupTableMenu ();

		void ExecCommand (const QString&, QString = QString ());
		bool QueryCommandState (const QString& cmd);

		void OpenFindReplace (bool findOnly);

		enum class ExpandMode
		{
			FullHTML,
			PartialHTML
		};
		QString ExpandCustomTags (QString, ExpandMode = ExpandMode::FullHTML) const;
		QString RevertCustomTags () const;

		void SyncHTMLToView () const;
	private slots:
		void handleBgColorSettings ();

		void handleLinkClicked (const QUrl&);
		void on_TabWidget__currentChanged (int);

		void setupJS ();

		void on_HTML__textChanged ();
		void updateActions ();

		void toggleView ();

		void handleCmd ();
		void handleInlineCmd ();
		void handleBgColor ();
		void handleFgColor ();
		void handleFont ();

		void handleInsertTable ();
		void handleInsertRow ();
		void handleInsertColumn ();
		void handleRemoveRow ();
		void handleRemoveColumn ();

		void handleInsertLink ();
		void handleInsertImage ();
		void handleCollectionImageChosen ();
		void handleInsertImageFromCollection ();

		void handleFind ();
		void handleReplace ();
	signals:
		void textChanged () override;
	};
}
}
