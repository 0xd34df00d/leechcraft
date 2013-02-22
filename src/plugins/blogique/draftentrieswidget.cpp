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

#include "draftentrieswidget.h"
#include <stdexcept>
#include <QStandardItemModel>
#include <QMessageBox>
#include <util/util.h>
#include "core.h"
#include "entriesfilterproxymodel.h"
#include "localstorage.h"
#include "utils.h"
#include "xmlsettingsmanager.h"
#include "interfaces/blogique/ibloggingplatform.h"

namespace LeechCraft
{
namespace Blogique
{
	DraftEntriesWidget::DraftEntriesWidget (QWidget *parent, Qt::WindowFlags f)
	: QWidget (parent, f)
	, Account_ (0)
	, DraftEntriesModel_ (new QStandardItemModel (this))
	, FilterProxyModel_ (new EntriesFilterProxyModel (this))
	{
		Ui_.setupUi (this);

		if (!Ui_.DraftEntriesCalendarSplitter_->
				restoreState (XmlSettingsManager::Instance ()
					.property ("DraftEntriesCalendarSplitterPosition")
						.toByteArray ()))
			Ui_.DraftEntriesCalendarSplitter_->setStretchFactor (1, 4);

		connect (Ui_.DraftEntriesCalendarSplitter_,
				SIGNAL (splitterMoved (int, int)),
				this,
				SLOT (saveSplitterPosition (int, int)));

		connect (Ui_.DraftEntriesCalendar_,
				SIGNAL (activated (QDate)),
				this,
				SLOT (loadDraftsByDate (QDate)));

		QAction *openDraftEntryInNewTab = new QAction (tr ("Open in new tab"), this);
		QAction *openDraftEntryInCurrentTab = new QAction (tr ("Open here"), this);
		QAction *showAllEntries = new QAction (tr ("Show all entries"), this);

		DraftEntriesModel_->setHorizontalHeaderLabels ({ tr ("Date"), tr ("Name") });
		FilterProxyModel_->setSourceModel (DraftEntriesModel_);
		Ui_.DraftEntriesView_->setModel (FilterProxyModel_);
		connect (openDraftEntryInCurrentTab,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenDraftEntryInCurrentTab ()));
		connect (openDraftEntryInNewTab,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenDraftEntryInNewTab ()));
		connect (showAllEntries,
				SIGNAL (triggered ()),
				this,
				SLOT (handleShowAllEntries ()));
		Ui_.DraftEntriesView_->setContextMenuPolicy (Qt::ActionsContextMenu);
		Ui_.DraftEntriesView_->addActions ({ openDraftEntryInNewTab,
				openDraftEntryInCurrentTab,
				Util::CreateSeparator (Ui_.DraftEntriesView_),
				showAllEntries });
	}

	QString DraftEntriesWidget::GetName () const
	{
		return tr ("Drafts");
	}

	void DraftEntriesWidget::SetAccount (IAccount *account)
	{
		Account_ = account;
		BloggingPLatform_ = qobject_cast<IBloggingPlatform*> (Account_->GetParentBloggingPlatform ());
	}

	void DraftEntriesWidget::LoadDraftEntries ()
	{
		if (!Account_)
			return;

		QList<Entry> entries;
		try
		{
			//TODO
// 			entries = Core::Instance ().GetStorage ()->
// 					GetDraftEntries (Account_->GetAccountID (),
// 							DraftStorage::Mode::ShortMode);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching short drafts"
					<< e.what ();
		}

		FillView (entries);
		FillStatistic ();
	}

	Entry DraftEntriesWidget::LoadFullEntry (qint64 id)
	{
		if (!Account_)
			return Entry ();

		try
		{
			return Entry ();
			//TODO
// 			return Core::Instance ().GetStorage ()->
// 					GetFullDraftEntry (Account_->GetAccountID (), id);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "error fetching full local entry"
				<< e.what ();
			return Entry ();
		}
	}

	void DraftEntriesWidget::FillView (const QList<Entry>& entries)
	{
		DraftEntriesModel_->removeRows (0, DraftEntriesModel_->rowCount());
		for (const auto& entry : entries)
		{
			const auto& items = Utils::CreateEntriesViewRow (entry);
			if (items.isEmpty ())
				continue;

			DraftEntriesModel_->appendRow (items);
			Item2Entry_ [items.first ()] = entry;
		}
		Ui_.DraftEntriesView_->resizeColumnToContents (0);
	}

	void DraftEntriesWidget::FillStatistic ()
	{
		if (!Account_)
			return;

		QMap<QDate, int> statistic;
		try
		{
			//TODO
// 			statistic = Core::Instance ().GetStorage ()->
// 					GetDraftEntriesCountByDate (Account_->GetAccountID ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		Ui_.DraftEntriesCalendar_->SetStatistic (statistic);
	}

	void DraftEntriesWidget::RemoveDraftEntry (qint64 id)
	{
		if (!Account_)
			return;

		try
		{
// 			Core::Instance ().GetStorage ()->
// 					RemoveDraftEntry (Account_->GetAccountID (), id);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error removing draft"
					<< e.what ();
		}
	}

	void DraftEntriesWidget::clear ()
	{
		FillView (QList<Entry> ());
	}

	void DraftEntriesWidget::saveSplitterPosition (int pos, int index)
	{
		XmlSettingsManager::Instance ()
				.setProperty ("DraftEntriesCalendarSplitterPosition",
						Ui_.DraftEntriesCalendarSplitter_->saveState ());
	}

	void DraftEntriesWidget::loadDraftsByDate (const QDate& date)
	{
		if (!Account_)
			return;

		QList<Entry> entries;
		try
		{
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

	void DraftEntriesWidget::FillCurrentTab (const QModelIndex& index)
	{
		auto sourceIndex = index.isValid () ?
			FilterProxyModel_->mapToSource (index) :
			FilterProxyModel_->mapToSource (Ui_.DraftEntriesView_->currentIndex ());

		if (!sourceIndex.isValid ())
			return;

		if (!Account_)
			return;

		sourceIndex = sourceIndex.sibling (sourceIndex.row (),
				Utils::EntriesViewColumns::Date);
		Entry e = Item2Entry_ [DraftEntriesModel_->itemFromIndex (sourceIndex)];
		if (e.IsEmpty ())
			return;

		e.EntryType_ = EntryType::BlogEntry;
		emit fillCurrentWidgetWithDraftEntry (e);
	}

	void DraftEntriesWidget::handleOpenDraftEntryInCurrentTab (const QModelIndex& index)
	{
		FillCurrentTab (index);
	}

	void DraftEntriesWidget::handleOpenDraftEntryInNewTab (const QModelIndex& index)
	{
		QModelIndex idx = index.isValid () ?
				index :
				Ui_.DraftEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		if (!Account_)
			return;

		idx = idx.sibling (idx.row (), Utils::EntriesViewColumns::Date);
		Entry e = LoadFullEntry (idx.data (Utils::EntryIdRole::DBIdRole)
				.toLongLong ());
		if (e.IsEmpty ())
			return;

		e.EntryType_ = EntryType::BlogEntry;
		emit fillNewWidgetWithDraftEntry (e, Account_->GetAccountID ());
	}

	void DraftEntriesWidget::on_DraftEntriesFilter__textChanged (const QString& text)
	{
		FilterProxyModel_->setFilterFixedString (text);
	}

	void DraftEntriesWidget::handleShowAllEntries ()
	{
		QList<Entry> entries;
		try
		{
// 			entries = Core::Instance ().GetStorage ()->
// 					GetDraftEntries (Account_->GetAccountID (),
// 							DraftStorage::Mode::ShortMode);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		FillView (entries);
	}

	void DraftEntriesWidget::on_RemoveDraftEntry__released ()
	{
		QModelIndex idx = Ui_.DraftEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		if (XmlSettingsManager::Instance ()
				.Property ("ConfirmDraftEntryRemoving", true).toBool ())
		{
			QMessageBox mbox (QMessageBox::Question,
					"LeechCraft",
					tr ("Do you want to delete selected entry?"),
					QMessageBox::Yes | QMessageBox::No,
					this);
			mbox.setDefaultButton (QMessageBox::No);

			QPushButton always (tr ("Always"));
			mbox.addButton (&always, QMessageBox::AcceptRole);

			if (mbox.exec () == QMessageBox::No)
				return;
			else if (mbox.clickedButton () == &always)
				XmlSettingsManager::Instance ()
						.setProperty ("ConfirmDraftEntryRemoving", false);
		}

		idx = idx.sibling (idx.row (), Utils::EntriesViewColumns::Date);
		RemoveDraftEntry (idx.data (Utils::EntryIdRole::DBIdRole).toLongLong ());
		DraftEntriesModel_->removeRow (idx.row ());
	}

	void DraftEntriesWidget::on_PublishDraftEntry__released ()
	{

	}

	void DraftEntriesWidget::on_DraftEntriesView__doubleClicked (const QModelIndex& index)
	{
		XmlSettingsManager::Instance ()
				.property ("OpenEntryByDblClick").toString () == "CurrentTab" ?
			handleOpenDraftEntryInCurrentTab (index) :
			handleOpenDraftEntryInNewTab (index);
	}

}
}