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
#include <interfaces/itexteditor.h>
#include <interfaces/iadvancedhtmleditor.h>
#include <interfaces/iwkfontssettable.h>
#include "ui_richeditorwidget.h"

class QToolBar;

namespace LC::LHTR
{
	class RichEditorWidget final : public QWidget
								 , public IEditorWidget
								 , public IAdvancedHTMLEditor
								 , public IWkFontsSettable
	{
		Q_OBJECT
		Q_INTERFACES (IEditorWidget IAdvancedHTMLEditor IWkFontsSettable)

		Ui::RichEditorWidget Ui_;

		QToolBar * const ViewBar_;

		QHash<QString, CustomTag> CustomTags_;
		QMap<CustomTag::TagType, QByteArray> CustomTagNames_;

		bool HTMLDirty_ = false;

		QAction *FindAction_;
		QAction *ReplaceAction_;

		QList<QPair<QStringView, QAction*>> HtmlActions_;
	public:
		explicit RichEditorWidget (QWidget* = nullptr);

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

		void ExecCommand (QStringView, QStringView = {});
		bool QueryCommandState (QStringView cmd);

		void OpenFindReplace (bool findOnly);

		enum class ExpandMode
		{
			FullHTML,
			PartialHTML
		};
		QString ExpandCustomTags (QString, ExpandMode = ExpandMode::FullHTML) const;
		QString RevertCustomTags (const QString&) const;

		QVariant ExecJSBlocking (const QString&);

		void SyncTabs (int chosenIdx);
		void SyncHTMLToView () const;

		void HandleHtmlCmdAction (QStringView cmd, QStringView args);
		void HandleInlineCmd (QStringView cmd, const QVariantMap& args = {});

		void HandleFont ();
		void HandleInsertLink ();
		void HandleInsertImage ();

		void HandleInsertTable ();
		void HandleInsertRow (int);
		void HandleInsertColumn (int);
		void HandleRemoveRow ();
		void HandleRemoveColumn ();
	private slots:
		void handleCollectionImageChosen ();
	signals:
		void textChanged () override;
	};
}
