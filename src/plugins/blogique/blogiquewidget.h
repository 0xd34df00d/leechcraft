/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include <interfaces/ihavetabs.h>
#include <interfaces/ihaverecoverabletabs.h>
#include "interfaces/blogique/iaccount.h"
#include "ui_blogiquewidget.h"

class QStandardItem;
class QStandardItemModel;
class IEditorWidget;
class QToolBar;
class QComboBox;
class QProgressBar;
class QQuickWidget;

namespace LC
{
namespace Blogique
{
	class IBlogiqueSideWidget;
	class IAccount;
	class DraftEntriesWidget;
	class BlogEntriesWidget;
	class TagsProxyModel;
	class CommentsWidget;

	class BlogiqueWidget : public QWidget
						, public ITabWidget
						, public IRecoverableTab
	{
		Q_OBJECT
		Q_INTERFACES (ITabWidget IRecoverableTab)

		enum BlogiqueSideWidgets
		{
			PostOptionsWidget = 2
		};

		static QObject *S_ParentMultiTabs_;

		Ui::BlogiqueWidget Ui_;
		QQuickWidget * const TagsCloud_;
		QQuickWidget * const Tags_;

		IEditorWidget *PostEdit_ = nullptr;
		QWidget *PostEditWidget_ = nullptr;
		QToolBar *ToolBar_;
		QToolBar *ProgressToolBar_;
		QAction *AccountsBoxAction_ = nullptr;
		QComboBox *AccountsBox_;
		QComboBox *PostTargetBox_ = nullptr;
		QAction *PostTargetAction_ = nullptr;
		QAction *ProgressBarLabelAction_ = nullptr;
		QLabel *ProgressBarLabel_ = nullptr;
		QAction *ProgressBarAction_ = nullptr;
		QList<QAction*> InlineTagInserters_;

		DraftEntriesWidget *DraftEntriesWidget_;
		BlogEntriesWidget *BlogEntriesWidget_;
		CommentsWidget *CommentsWidget_;
		QHash<int, IAccount*> Id2Account_;
		int PrevAccountId_ = -1;
		QList<QWidget*> SidePluginsWidgets_;

		EntryType EntryType_ = EntryType::None;
		qint64 EntryId_ = -1;
		QUrl EntryUrl_;

		bool EntryChanged_ = false;

		TagsProxyModel *TagsProxyModel_;
		QStandardItemModel *TagsModel_;

	public:
		enum TagRoles
		{
			TagFrequency = Qt::UserRole + 1
		};

		BlogiqueWidget (QWidget *parent = nullptr);

		QObject* ParentMultiTabs ();
		TabClassInfo GetTabClassInfo () const;
		QToolBar* GetToolBar () const;
		void Remove ();

		static void SetParentMultiTabs (QObject *tab);

		QByteArray GetTabRecoverData () const;
		QString GetTabRecoverName () const;
		QIcon GetTabRecoverIcon () const;
		void FillWidget (const Entry& e, const QByteArray& accId = QByteArray ());
	private:
		void SetTextEditor ();
		void SetToolBarActions ();
		void SetDefaultSideWidgets ();
		void PrepareQmlWidgets ();
		void RemovePostingTargetsWidget ();

		void SetPostDate (const QDateTime& dt);
		QDateTime GetPostDate () const;

		void SetPostTags (const QStringList& tags);
		QStringList GetPostTags () const;

		void ClearEntry ();

		Entry GetCurrentEntry (bool interactive = false) const;

		void ShowProgress (const QString& labelText = QString ());

	public slots:
		void handleAutoSave ();
		void handleEntryPosted ();
		void handleEntryRemoved ();
		void handleRequestEntriesBegin ();
		void handleRequestEntriesEnd ();
		void handleTagsUpdated (const QHash<QString, int>& tags);
		void handleInsertTag (const QString& tag);
		void handleGotError (int errorCode, const QString& errorString,
				const QString& localizedErrorString);
		void handleAccountAdded (QObject *acc);
		void handleAccountRemoved (QObject *acc);

	private slots:
		void handleCurrentAccountChanged (int id);
		void fillCurrentTabWithEntry (const Entry& entry);
		void fillNewTabWithEntry (const Entry& entry, const QByteArray& accountId);

		void handleEntryChanged (const QString& str = QString ());
		void handleRemovingEntryBegin ();

		void newEntry ();
		void saveEntry (const Entry& e = Entry ());
		void saveNewEntry (const Entry& e = Entry ());
		void submit (const Entry& e = Entry ());
		void submitTo (const Entry& e = Entry ());
		void on_SideWidget__dockLocationChanged (Qt::DockWidgetArea area);
		void on_ShowProfile__triggered ();
		void on_CurrentTime__released ();
		void on_SelectTags__toggled (bool checked);
		void handleTagTextChanged (const QString& text);
		void handleTagRemoved (const QString& tag);
		void handleTagAdded (const QString& tag);
		void on_OpenInBrowser__triggered ();
		void on_PreviewPost__triggered ();

	signals:
		void removeTab (QWidget *tab);
		void addNewTab (const QString& name, QWidget *tab);
		void changeTabName (QWidget *content, const QString& name);

		void tabRecoverDataChanged ();

		void tagSelected (const QString& tag);
	};
}
}
