/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
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

#include "blogentrieswidget.h"
#include <stdexcept>
#include <QMessageBox>
#include <QStandardItemModel>
#include "entriesfilterproxymodel.h"
#include "xmlsettingsmanager.h"
#include "core.h"
#include "utils.h"

namespace LeechCraft
{
namespace Blogique
{
	BlogEntriesWidget::BlogEntriesWidget (QWidget *parent, Qt::WindowFlags f)
	: QWidget (parent, f)
	, Account_ (0)
	, BlogEntriesModel_ (new QStandardItemModel (this))
	, FilterProxyModel_ (new EntriesFilterProxyModel (this))
	{
		Ui_.setupUi (this);

		if (!Ui_.BlogEntriesCalendarSplitter_->
				restoreState (XmlSettingsManager::Instance ()
					.property ("LocalEntriesCalendarSplitterPosition")
					.toByteArray ()))
			Ui_.BlogEntriesCalendarSplitter_->setStretchFactor (1, 4);

		connect (Ui_.BlogEntriesCalendarSplitter_,
				SIGNAL (splitterMoved (int, int)),
				this,
				SLOT (saveSplitterPosition (int, int)));

		connect (Ui_.BlogEntriesCalendar_,
				SIGNAL (activated (QDate)),
				this,
				SLOT (loadPostsByDate (QDate)));

		connect (Ui_.CalendarVisibility_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleCalendarVisibilityChanged (bool)));

		BlogEntriesModel_->setHorizontalHeaderLabels ({ tr ("Date"), tr ("Name") });
		FilterProxyModel_->setSourceModel (BlogEntriesModel_);
		Ui_.BlogEntriesView_->setModel (FilterProxyModel_);

		QAction *openBlogEntryInNewTab = new QAction (tr ("Open in new tab"), this);
		QAction *openBlogEntryInCurrentTab = new QAction (tr ("Open here"), this);
		connect (openBlogEntryInCurrentTab,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenBlogEntryInCurrentTab ()));
		connect (openBlogEntryInNewTab,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenBlogEntryInNewTab ()));

		Ui_.BlogEntriesView_->setContextMenuPolicy (Qt::ActionsContextMenu);
		Ui_.BlogEntriesView_->addActions ({ openBlogEntryInNewTab,
				openBlogEntryInCurrentTab });

		Ui_.CalendarVisibility_->setChecked (XmlSettingsManager::Instance ()
				.Property ("ShowBlogPostsCalendar", true).toBool ());
	}

	QString BlogEntriesWidget::GetName () const
	{
		return tr ("Blog entries");
	}

	void BlogEntriesWidget::SetAccount (IAccount *account)
	{
		for (auto act : LoadActions_)
			Ui_.LoadBlogEntries_->removeAction (act);
		LoadActions_.clear ();

		Account_ = account;
		connect (Account_->GetObject (),
				SIGNAL (gotBlogStatistics (QMap<QDate, int>)),
				this,
				SLOT (fillStatistic (QMap<QDate, int>)),
				Qt::UniqueConnection);
		connect (Account_->GetObject (),
				SIGNAL (gotEntries (QList<Entry>)),
				this,
				SLOT (fillView (QList<Entry>)),
				Qt::UniqueConnection);

		LoadActions_ = account->GetUpdateActions ();
		Ui_.LoadBlogEntries_->addActions (LoadActions_);

		Account_->RequestStatistics ();
	}

	Entry BlogEntriesWidget::GetEntry (const QModelIndex& index)
	{
		auto sourceIndex = index.isValid () ?
			FilterProxyModel_->mapToSource (index) :
			FilterProxyModel_->mapToSource (Ui_.BlogEntriesView_->currentIndex ());

		if (!sourceIndex.isValid ())
			return Entry ();

		if (!Account_)
			return Entry ();

		sourceIndex = sourceIndex.sibling (sourceIndex.row (),
				Utils::EntriesViewColumns::Date);
		Entry e = Item2Entry_ [BlogEntriesModel_->itemFromIndex (sourceIndex)];
		if (e.IsEmpty ())
			return Entry ();

		e.EntryType_ = EntryType::BlogEntry;

		return e;
	}

	void BlogEntriesWidget::FillCurrentTab (const QModelIndex& index)
	{
		const auto& e = GetEntry (index);
		if (!e.IsEmpty ())
			emit fillCurrentWidgetWithBlogEntry (e);
	}

	void BlogEntriesWidget::clear ()
	{
		fillView (QList<Entry> ());
	}

	void BlogEntriesWidget::fillView (const QList<Entry>& entries)
	{
		BlogEntriesModel_->removeRows (0, BlogEntriesModel_->rowCount ());
		Item2Entry_.clear ();
		for (const auto& entry : entries)
		{
			const auto& items = Utils::CreateEntriesViewRow (entry);
			if (items.isEmpty ())
				continue;

			BlogEntriesModel_->appendRow (items);
			Item2Entry_ [items.first ()] = entry;
		}
		Ui_.BlogEntriesView_->resizeColumnToContents (0);
	}

	void BlogEntriesWidget::fillStatistic (const QMap<QDate, int>& statistics)
	{
		if (!Account_)
			return;

		Ui_.BlogEntriesCalendar_->SetStatistic (statistics);
	}

	void BlogEntriesWidget::saveSplitterPosition (int pos, int index)
	{
		XmlSettingsManager::Instance ()
				.setProperty ("BlogEntriesCalendarSplitterPosition",
					Ui_.BlogEntriesCalendarSplitter_->saveState ());
	}

	void BlogEntriesWidget::loadPostsByDate (const QDate& date)
	{
		if (!Account_)
			return;

		Account_->GetEntriesByDate (date);
	}

	void BlogEntriesWidget::handleOpenBlogEntryInCurrentTab (const QModelIndex& index)
	{
		FillCurrentTab (index);
	}

	void BlogEntriesWidget::handleOpenBlogEntryInNewTab (const QModelIndex& index)
	{
		const auto& e = GetEntry (index);
		if (!e.IsEmpty ())
			emit fillNewWidgetWithBlogEntry (e, Account_->GetAccountID ());
	}

	void BlogEntriesWidget::on_BlogEntriesFilter__textChanged (const QString& text)
	{
		FilterProxyModel_->setFilterFixedString (text);
	}

	void BlogEntriesWidget::on_RemoveBlogEntry__released ()
	{
		const auto& idx = Ui_.BlogEntriesView_->currentIndex ();
		auto sourceIndex = FilterProxyModel_->mapToSource (idx);
		if (!idx.isValid () ||
				!sourceIndex.isValid ())
			return;

		if (!Account_)
			return;

		sourceIndex = sourceIndex.sibling (sourceIndex.row (),
				Utils::EntriesViewColumns::Date);
		const Entry& e = Item2Entry_ [BlogEntriesModel_->itemFromIndex (sourceIndex)];
		if (e.IsEmpty ())
			return;

		if (QMessageBox::question (this, "LeechCraft",
				tr ("Are you sure you want to delete entry %1 from the blog?")
						.arg ("<em>" + e.Subject_ + "</em>"),
				QMessageBox::Ok | QMessageBox::Cancel,
				QMessageBox::Cancel) == QMessageBox::Ok)
			Account_->RemoveEntry (e);
	}

	void BlogEntriesWidget::on_BlogEntriesView__doubleClicked (const QModelIndex& index)
	{
		XmlSettingsManager::Instance ()
				.property ("OpenEntryByDblClick").toString () == "CurrentTab" ?
			handleOpenBlogEntryInCurrentTab (index) :
			handleOpenBlogEntryInNewTab (index);
	}

	void BlogEntriesWidget::handleCalendarVisibilityChanged (bool visible)
	{
		XmlSettingsManager::Instance ().setProperty ("ShowBlogPostsCalendar",
				visible);
	}

}
}