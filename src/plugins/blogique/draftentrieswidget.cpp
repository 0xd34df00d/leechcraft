/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2013  Oleg Linkin
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "draftentrieswidget.h"
#include <stdexcept>
#include <QStandardItemModel>
#include <QMessageBox>
#include <util/util.h>
#include "interfaces/blogique/ibloggingplatform.h"
#include "core.h"
#include "entriesfilterproxymodel.h"
#include "storagemanager.h"
#include "utils.h"
#include "xmlsettingsmanager.h"
#include "submittodialog.h"

namespace LC
{
namespace Blogique
{
	DraftEntriesWidget::DraftEntriesWidget (QWidget *parent)
	: QWidget (parent)
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

		connect (Ui_.CalendarVisibility_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (handleCalendarVisibilityChanged (bool)));

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
				SLOT (loadDraftEntries ()));
		Ui_.DraftEntriesView_->setContextMenuPolicy (Qt::ActionsContextMenu);
		Ui_.DraftEntriesView_->addActions ({ openDraftEntryInNewTab,
				openDraftEntryInCurrentTab,
				Util::CreateSeparator (Ui_.DraftEntriesView_),
				showAllEntries });

		Ui_.CalendarVisibility_->setChecked (XmlSettingsManager::Instance ()
				.Property ("ShowDraftCalendar", true).toBool ());
	}

	QString DraftEntriesWidget::GetName () const
	{
		return tr ("Drafts");
	}

	namespace
	{
		Entry LoadFullEntry (qint64 id)
		{
			try
			{
				return Core::Instance ().GetStorageManager ()->GetFullDraft (id);
			}
			catch (const std::runtime_error& e)
			{
				qWarning () << Q_FUNC_INFO
						<< "error fetching full local entry"
						<< e.what ();
				return Entry ();
			}
		}
	}

	void DraftEntriesWidget::FillView (const QList<Entry>& entries)
	{
		DraftEntriesModel_->removeRows (0, DraftEntriesModel_->rowCount ());
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
		QMap<QDate, int> statistic;
		try
		{
			statistic = Core::Instance ().GetStorageManager ()->
					GetDraftsCountByDate ();
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
		try
		{
			Core::Instance ().GetStorageManager ()->RemoveDraft (id);
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

	void DraftEntriesWidget::loadDraftEntries ()
	{
		QList<Entry> entries;
		try
		{
			entries = Core::Instance ().GetStorageManager ()->
					GetDrafts (StorageManager::Mode::ShortMode);
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

	void DraftEntriesWidget::saveSplitterPosition (int, int)
	{
		XmlSettingsManager::Instance ()
				.setProperty ("DraftEntriesCalendarSplitterPosition",
						Ui_.DraftEntriesCalendarSplitter_->saveState ());
	}

	void DraftEntriesWidget::loadDraftsByDate (const QDate& date)
	{
		QList<Entry> entries;
		try
		{
			entries = Core::Instance ().GetStorageManager ()->
					GetDraftsByDate (date);
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

		sourceIndex = sourceIndex.sibling (sourceIndex.row (),
				Utils::EntriesViewColumns::Date);
		Entry e = Item2Entry_ [DraftEntriesModel_->itemFromIndex (sourceIndex)];
		if (e.IsEmpty ())
			return;

		e.EntryType_ = EntryType::Draft;
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

		idx = idx.sibling (idx.row (), Utils::EntriesViewColumns::Date);
		Entry e = LoadFullEntry (idx.data (Utils::EntryIdRole::DBIdRole)
				.toLongLong ());
		if (e.IsEmpty ())
			return;

		e.EntryType_ = EntryType::Draft;
		emit fillNewWidgetWithDraftEntry (e);
	}

	void DraftEntriesWidget::on_DraftEntriesFilter__textChanged (const QString& text)
	{
		FilterProxyModel_->setFilterFixedString (text);
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
		FillStatistic ();
	}

	void DraftEntriesWidget::on_PublishDraftEntry__released ()
	{
		QModelIndex idx = Ui_.DraftEntriesView_->currentIndex ();
		if (!idx.isValid ())
			return;
		idx = idx.sibling (idx.row (), Utils::EntriesViewColumns::Date);

		SubmitToDialog dlg;
		if (dlg.exec () == QDialog::Rejected)
			return;

		for (const auto& pair : dlg.GetPostingTargets ())
		{
			auto e = Item2Entry_ [DraftEntriesModel_->itemFromIndex (idx)];
			e.Target_ = pair.second;
			pair.first->submit (e);
		}
	}

	void DraftEntriesWidget::on_DraftEntriesView__doubleClicked (const QModelIndex& index)
	{
		XmlSettingsManager::Instance ()
				.property ("OpenEntryByDblClick").toString () == "CurrentTab" ?
			handleOpenDraftEntryInCurrentTab (index) :
			handleOpenDraftEntryInNewTab (index);
	}

	void DraftEntriesWidget::handleCalendarVisibilityChanged (bool visible)
	{
		XmlSettingsManager::Instance ().setProperty ("ShowDraftCalendar", visible);
	}

}
}
