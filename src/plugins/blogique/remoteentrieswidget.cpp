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

#include "remoteentrieswidget.h"
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
	RemoteEntriesWidget::RemoteEntriesWidget (QWidget *parent, Qt::WindowFlags f)
	: QWidget (parent, f)
	, Account_ (0)
	, RemoteEntriesModel_ (new QStandardItemModel (this))
	, FilterProxyModel_ (new EntriesFilterProxyModel (this))
	{
		Ui_.setupUi (this);

		if (!Ui_.RemoteEntriesCalendarSplitter_->
				restoreState (XmlSettingsManager::Instance ()
					.property ("LocalEntriesCalendarSplitterPosition")
					.toByteArray ()))
			Ui_.RemoteEntriesCalendarSplitter_->setStretchFactor (1, 4);

		connect (Ui_.RemoteEntriesCalendarSplitter_,
				SIGNAL (splitterMoved (int, int)),
				this,
				SLOT (saveSplitterPosition (int, int)));

		connect (Ui_.RemoteEntriesCalendar_,
				SIGNAL (activated (QDate)),
				this,
				SLOT (loadPostsByDate (QDate)));

		RemoteEntriesModel_->setHorizontalHeaderLabels ({ tr ("Date"), tr ("Name") });
		FilterProxyModel_->setSourceModel (RemoteEntriesModel_);
		Ui_.RemoteEntriesView_->setModel (FilterProxyModel_);

		QAction *openRemoteEntryInNewTab = new QAction (tr ("Open in new tab"), this);
		QAction *openRemoteEntryInCurrentTab = new QAction (tr ("Open here"), this);
		connect (openRemoteEntryInCurrentTab,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenRemoteEntryInCurrentTab ()));
		connect (openRemoteEntryInNewTab,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenRemoteEntryInNewTab ()));

		Ui_.RemoteEntriesView_->setContextMenuPolicy (Qt::ActionsContextMenu);
		Ui_.RemoteEntriesView_->addActions ({ openRemoteEntryInNewTab,
				openRemoteEntryInCurrentTab });

		connect (&Core::Instance (),
				SIGNAL (gotEntries (QObject*, QList<Entry>)),
				this,
				SLOT (handleGotEntries (QObject*, QList<Entry>)));
	}

	QString RemoteEntriesWidget::GetName () const
	{
		return tr ("Remote entries");
	}

	void RemoteEntriesWidget::SetAccount (IAccount *account)
	{
		for (auto act : LoadActions_)
			Ui_.LoadRemoteEntries_->removeAction (act);
		LoadActions_.clear ();

		Account_ = account;

		auto actions = account->GetUpdateActions ();
		Ui_.LoadRemoteEntries_->addActions (actions);
		LoadActions_ = actions;
	}

	Entry RemoteEntriesWidget::LoadFullEntry (qint64 id)
	{
		if (!Account_)
			return Entry ();

		try
		{
			return Core::Instance ().GetStorage ()->
					GetEntry (Account_->GetAccountID (), id);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching full local entry"
					<< e.what ();
			return Entry ();
		}
	}

	void RemoteEntriesWidget::FillView (const QList<Entry>& entries)
	{
		RemoteEntriesModel_->removeRows (0, RemoteEntriesModel_->rowCount());
		Item2Entry_.clear ();
		for (const auto& entry : entries)
		{
			const auto& items = Utils::CreateEntriesViewRow (entry);
			if (items.isEmpty ())
				continue;

			RemoteEntriesModel_->appendRow (items);
			Item2Entry_ [items.first ()] = entry;
		}
		Ui_.RemoteEntriesView_->resizeColumnToContents (0);
	}

	void RemoteEntriesWidget::FillStatistic ()
	{
		if (!Account_)
			return;

		QMap<QDate, int> statistic;
		try
		{
			statistic = Core::Instance ().GetStorage ()->
					GetEntriesCountByDate (Account_->GetAccountID ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		Ui_.RemoteEntriesCalendar_->SetStatistic (statistic);
	}

	void RemoteEntriesWidget::FillCurrentTab (const QModelIndex& index)
	{
		auto sourceIndex = index.isValid () ?
			FilterProxyModel_->mapToSource (index) :
			FilterProxyModel_->mapToSource (Ui_.RemoteEntriesView_->currentIndex ());

		if (!sourceIndex.isValid ())
			return;

		if (!Account_)
			return;

		sourceIndex = sourceIndex.sibling (sourceIndex.row (),
				Utils::EntriesViewColumns::Date);
		Entry e = Item2Entry_ [RemoteEntriesModel_->itemFromIndex (sourceIndex)];
		if (e.IsEmpty ())
			return;

		e.EntryType_ = EntryType::RemoteEntry;
		emit fillCurrentWidgetWithRemoteEntry (e);
	}

	void RemoteEntriesWidget::clear ()
	{
		FillView (QList<Entry> ());
	}

	void RemoteEntriesWidget::saveSplitterPosition (int pos, int index)
	{
		XmlSettingsManager::Instance ()
				.setProperty ("RemoteEntriesCalendarSplitterPosition",
					Ui_.RemoteEntriesCalendarSplitter_->saveState ());
	}

	void RemoteEntriesWidget::loadPostsByDate (const QDate& date)
	{
		if (!Account_)
			return;

		QList<Entry> entries;
		try
		{
			entries = Core::Instance ().GetStorage ()->
					GetEntriesByDate (Account_->GetAccountID (), date);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		FillView (entries);
	}

	void RemoteEntriesWidget::handleOpenRemoteEntryInCurrentTab (const QModelIndex& index)
	{
		FillCurrentTab (index);
	}

	void RemoteEntriesWidget::handleOpenRemoteEntryInNewTab (const QModelIndex& index)
	{
		auto sourceIndex = index.isValid () ?
			FilterProxyModel_->mapToSource (index) :
			FilterProxyModel_->mapToSource (Ui_.RemoteEntriesView_->currentIndex ());

		if (!sourceIndex.isValid ())
			return;

		if (!Account_)
			return;

		sourceIndex = sourceIndex.sibling (sourceIndex.row (),
				Utils::EntriesViewColumns::Date);
		Entry e = Item2Entry_ [RemoteEntriesModel_->itemFromIndex (sourceIndex)];
		if (e.IsEmpty ())
			return;

		e.EntryType_ = EntryType::RemoteEntry;
		emit fillNewWidgetWithRemoteEntry (e, Account_->GetAccountID ());
	}

	void RemoteEntriesWidget::on_RemoteEntriesFilter__textChanged (const QString& text)
	{
		FilterProxyModel_->setFilterFixedString (text);
	}

	void RemoteEntriesWidget::on_RemoveRemoteEntry__released ()
	{
		const auto& idx = Ui_.RemoteEntriesView_->currentIndex ();
		auto sourceIndex = FilterProxyModel_->mapToSource (idx);
		if (!idx.isValid () ||
				!sourceIndex.isValid ())
			return;

		if (!Account_)
			return;

		sourceIndex = sourceIndex.sibling (sourceIndex.row (),
				Utils::EntriesViewColumns::Date);
		const Entry& e = Item2Entry_ [RemoteEntriesModel_->itemFromIndex (sourceIndex)];
		if (e.IsEmpty ())
			return;

		if (QMessageBox::question (this, "LeechCraft",
			tr ("Do you want to remove this entry from blog?"),
				QMessageBox::Ok | QMessageBox::Cancel,
		QMessageBox::Cancel) == QMessageBox::Ok)
			Account_->RemoveEntry (e);
	}

	void RemoteEntriesWidget::handleGotEntries (QObject *acc,
			const QList<Entry>& entries)
	{
		if (acc != Account_->GetObject ())
			return;

		FillView (entries);
	}

	void RemoteEntriesWidget::on_RemoteEntriesView__doubleClicked (const QModelIndex& index)
	{
		XmlSettingsManager::Instance ()
				.property ("OpenEntryByDblClick").toString () == "CurrentTab" ?
			handleOpenRemoteEntryInCurrentTab (index) :
			handleOpenRemoteEntryInNewTab (index);
	}
}
}