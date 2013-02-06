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

#include "localentrieswidget.h"
#include <stdexcept>
#include <QStandardItemModel>
#include <QMessageBox>
#include <util/util.h>
#include "core.h"
#include "entriesfilterproxymodel.h"
#include "localstorage.h"
#include "utils.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Blogique
{
	LocalEntriesWidget::LocalEntriesWidget (QWidget *parent, Qt::WindowFlags f)
	: QWidget (parent, f)
	, Account_ (0)
	, LocalEntriesModel_ (new QStandardItemModel (this))
	, FilterProxyModel_ (new EntriesFilterProxyModel (this))
	{
		Ui_.setupUi (this);

		if (!Ui_.LocalEntriesCalendarSplitter_->
				restoreState (XmlSettingsManager::Instance ()
					.property ("LocalEntriesCalendarSplitterPosition")
						.toByteArray ()))
			Ui_.LocalEntriesCalendarSplitter_->setStretchFactor (1, 4);

		connect (Ui_.LocalEntriesCalendarSplitter_,
				SIGNAL (splitterMoved (int, int)),
				this,
				SLOT (saveSplitterPosition (int, int)));

		connect (Ui_.LocalEntriesCalendar_,
				SIGNAL (activated (QDate)),
				this,
				SLOT (loadPostsByDate (QDate)));

		QAction *openLocalEntryInNewTab = new QAction (tr ("Open in new tab"), this);
		QAction *openLocalEntryInCurrentTab = new QAction (tr ("Open here"), this);
		QAction *showAllEntries = new QAction (tr ("Show all entries"), this);

		LocalEntriesModel_->setHorizontalHeaderLabels ({ tr ("Date"), tr ("Name") });
		FilterProxyModel_->setSourceModel (LocalEntriesModel_);
		Ui_.LocalEntriesView_->setModel (FilterProxyModel_);
		connect (openLocalEntryInCurrentTab,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenLocalEntryInCurrentTab ()));
		connect (openLocalEntryInNewTab,
				SIGNAL (triggered ()),
				this,
				SLOT (handleOpenLocalEntryInNewTab ()));
		connect (showAllEntries,
				SIGNAL (triggered ()),
				this,
				SLOT (handleShowAllEntries ()));
		Ui_.LocalEntriesView_->setContextMenuPolicy (Qt::ActionsContextMenu);
		Ui_.LocalEntriesView_->addActions ({ openLocalEntryInNewTab,
				openLocalEntryInCurrentTab,
				Util::CreateSeparator (Ui_.LocalEntriesView_),
				showAllEntries });
	}

	QString LocalEntriesWidget::GetName () const
	{
		return tr ("Local entries");
	}

	void LocalEntriesWidget::SetAccount (IAccount *account)
	{
		Account_ = account;
	}

	void LocalEntriesWidget::LoadLocalEntries ()
	{
		QList<Entry> entries;
		try
		{
			entries = Core::Instance ().GetStorage ()->
					GetLocalEntries (Account_->GetAccountID (),
							LocalStorage::Mode::ShortMode);
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

	Entry LocalEntriesWidget::LoadFullEntry (qint64 id)
	{
		if (!Account_)
			return Entry ();

		try
		{
			return Core::Instance ().GetStorage ()->
					GetFullLocalEntry (Account_->GetAccountID (), id);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
				<< "error fetching full local entry"
				<< e.what ();
			return Entry ();
		}
	}

	void LocalEntriesWidget::FillView (const QList<Entry>& entries)
	{
		LocalEntriesModel_->removeRows (0, LocalEntriesModel_->rowCount());
		for (const auto& entry : entries)
		{
			const auto& items = Utils::CreateEntriesViewRow (entry);
			if (items.isEmpty ())
				continue;

			LocalEntriesModel_->appendRow (items);
			Item2Entry_ [items.first ()] = entry;
		}
		Ui_.LocalEntriesView_->resizeColumnToContents (0);
	}

	void LocalEntriesWidget::FillStatistic ()
	{
		if (!Account_)
			return;

		QMap<QDate, int> statistic;
		try
		{
				statistic = Core::Instance ().GetStorage ()
						->GetLocalEntriesCountByDate (Account_->GetAccountID ());
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		Ui_.LocalEntriesCalendar_->SetStatistic (statistic);
	}

	void LocalEntriesWidget::RemoveLocalEntry (qint64 id)
	{
		if (!Account_)
			return;

		try
		{
			Core::Instance ().GetStorage ()->
					RemoveLocalEntry (Account_->GetAccountID (), id);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error removing draft"
					<< e.what ();
		}
	}

	void LocalEntriesWidget::clear ()
	{
		FillView (QList<Entry> ());
	}

	void LocalEntriesWidget::saveSplitterPosition (int pos, int index)
	{
		XmlSettingsManager::Instance ()
				.setProperty ("LocalEntriesCalendarSplitterPosition",
						Ui_.LocalEntriesCalendarSplitter_->saveState ());
	}

	void LocalEntriesWidget::loadPostsByDate (const QDate& date)
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

	void LocalEntriesWidget::handleOpenLocalEntryInCurrentTab (const QModelIndex& index)
	{
		QModelIndex idx = index.isValid () ?
			index :
			Ui_.LocalEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		idx = idx.sibling (idx.row (), Utils::EntriesViewColumns::Date);
		const Entry& e = LoadFullEntry (idx.data (Utils::EntryIdRole::DBIdRole)
				.toLongLong ());

		emit fillCurrentWidgetWithLocalEntry (e);
	}

	void LocalEntriesWidget::handleOpenLocalEntryInNewTab (const QModelIndex& index)
	{
		QModelIndex idx = index.isValid () ?
				index :
				Ui_.LocalEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		if (!Account_)
			return;

		idx = idx.sibling (idx.row (), Utils::EntriesViewColumns::Date);
		const Entry& e = LoadFullEntry (idx.data (Utils::EntryIdRole::DBIdRole)
				.toLongLong ());

		emit fillNewWidgetWithLocalEntry (e, Account_->GetAccountID ());
	}

	void LocalEntriesWidget::on_LocalEntriesFilter__textChanged (const QString& text)
	{
		FilterProxyModel_->setFilterFixedString (text);
	}

	void LocalEntriesWidget::handleShowAllEntries ()
	{
		QList<Entry> entries;
		try
		{
			entries = Core::Instance ().GetStorage ()->
					GetLocalEntries (Account_->GetAccountID (),
							LocalStorage::Mode::ShortMode);
		}
		catch (const std::runtime_error& e)
		{
			qWarning () << Q_FUNC_INFO
					<< "error fetching entries"
					<< e.what ();
		}

		FillView (entries);
	}

	void LocalEntriesWidget::on_RemoveLocalEntry__released ()
	{
		QModelIndex idx = Ui_.LocalEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;

		if (XmlSettingsManager::Instance ()
				.Property ("ConfirmLocalEntryRemoving", true).toBool ())
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
						.setProperty ("ConfirmLocalEntryRemoving", false);
		}

		idx = idx.sibling (idx.row (), Utils::EntriesViewColumns::Date);
		RemoveLocalEntry (idx.data (Utils::EntryIdRole::DBIdRole).toLongLong ());
		LocalEntriesModel_->removeRow (idx.row ());
	}

	void LocalEntriesWidget::on_PublishLocalEntry__released ()
	{

	}

}
}