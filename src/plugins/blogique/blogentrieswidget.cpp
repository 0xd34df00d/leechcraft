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

		connect (&Core::Instance (),
				SIGNAL (gotEntries (QObject*, QList<Entry>)),
				this,
				SLOT (handleGotEntries (QObject*, QList<Entry>)));
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

		auto actions = account->GetUpdateActions ();
		Ui_.LoadBlogEntries_->addActions (actions);
		LoadActions_ = actions;
	}

	Entry BlogEntriesWidget::LoadFullEntry (qint64 id)
	{
		if (!Account_)
			return Entry ();

		try
		{
			//TODO
			return Entry ();
// 			return Core::Instance ().GetStorage ()->
// 					GetEntry (Account_->GetAccountID (), id);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching full local entry"
					<< e.what ();
			return Entry ();
		}
	}

	void BlogEntriesWidget::FillView (const QList<Entry>& entries)
	{
		BlogEntriesModel_->removeRows (0, BlogEntriesModel_->rowCount());
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

	void BlogEntriesWidget::FillStatistic ()
	{
		if (!Account_)
			return;

		QMap<QDate, int> statistic;
		try
		{
			//TODO
// 			statistic = Core::Instance ().GetStorage ()->
// 					GetEntriesCountByDate (Account_->GetAccountID ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		Ui_.BlogEntriesCalendar_->SetStatistic (statistic);
	}

	void BlogEntriesWidget::FillCurrentTab (const QModelIndex& index)
	{
		auto sourceIndex = index.isValid () ?
			FilterProxyModel_->mapToSource (index) :
			FilterProxyModel_->mapToSource (Ui_.BlogEntriesView_->currentIndex ());

		if (!sourceIndex.isValid ())
			return;

		if (!Account_)
			return;

		sourceIndex = sourceIndex.sibling (sourceIndex.row (),
				Utils::EntriesViewColumns::Date);
		Entry e = Item2Entry_ [BlogEntriesModel_->itemFromIndex (sourceIndex)];
		if (e.IsEmpty ())
			return;

		e.EntryType_ = EntryType::BlogEntry;
		emit fillCurrentWidgetWithBlogEntry (e);
	}

	void BlogEntriesWidget::clear ()
	{
		FillView (QList<Entry> ());
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

		QList<Entry> entries;
		try
		{
			//TODO
// 			entries = Core::Instance ().GetStorage ()->
// 					GetEntriesByDate (Account_->GetAccountID (), date);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		FillView (entries);
	}

	void BlogEntriesWidget::handleOpenBlogEntryInCurrentTab (const QModelIndex& index)
	{
		FillCurrentTab (index);
	}

	void BlogEntriesWidget::handleOpenBlogEntryInNewTab (const QModelIndex& index)
	{
		auto sourceIndex = index.isValid () ?
			FilterProxyModel_->mapToSource (index) :
			FilterProxyModel_->mapToSource (Ui_.BlogEntriesView_->currentIndex ());

		if (!sourceIndex.isValid ())
			return;

		if (!Account_)
			return;

		sourceIndex = sourceIndex.sibling (sourceIndex.row (),
				Utils::EntriesViewColumns::Date);
		Entry e = Item2Entry_ [BlogEntriesModel_->itemFromIndex (sourceIndex)];
		if (e.IsEmpty ())
			return;

		e.EntryType_ = EntryType::BlogEntry;
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
			tr ("Do you want to remove this entry from blog?"),
				QMessageBox::Ok | QMessageBox::Cancel,
		QMessageBox::Cancel) == QMessageBox::Ok)
			Account_->RemoveEntry (e);
	}

	void BlogEntriesWidget::handleGotEntries (QObject *acc,
			const QList<Entry>& entries)
	{
		if (acc != Account_->GetObject ())
			return;

		FillView (entries);
	}

	void BlogEntriesWidget::on_BlogEntriesView__doubleClicked (const QModelIndex& index)
	{
		XmlSettingsManager::Instance ()
				.property ("OpenEntryByDblClick").toString () == "CurrentTab" ?
			handleOpenBlogEntryInCurrentTab (index) :
			handleOpenBlogEntryInNewTab (index);
	}
}
}