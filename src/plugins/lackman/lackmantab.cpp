/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "lackmantab.h"
#include <QStringListModel>
#include <QShortcut>
#include <QToolBar>
#include <QMenu>
#include <util/shortcuts/shortcutmanager.h>
#include <util/tags/tagscompleter.h>
#include <util/util.h>
#include <util/gui/clearlineeditaddon.h>
#include <util/gui/lineeditbuttonmanager.h>
#include <interfaces/core/itagsmanager.h>
#include <interfaces/core/ipluginsmanager.h>
#include "core.h"
#include "typefilterproxymodel.h"
#include "stringfiltermodel.h"
#include "packagesmodel.h"
#include "packagesdelegate.h"
#include "pendingmanager.h"
#include "storage.h"
#include "updatesnotificationmanager.h"

Q_DECLARE_METATYPE (QModelIndex)

namespace LC
{
namespace LackMan
{
	LackManTab::LackManTab (Util::ShortcutManager *sm, const TabClassInfo& tabclass, QObject *parentPlugin)
	: TC_ (tabclass)
	, ParentPlugin_ (parentPlugin)
	, ShortcutMgr_ (sm)
	, TagsModel_ (new QStringListModel (this))
	, FilterString_ (new StringFilterModel (this))
	, TypeFilter_ (new TypeFilterProxyModel (this))
	{
		Ui_.setupUi (this);

		auto searchLineButtonMgr = new Util::LineEditButtonManager { Ui_.SearchLine_ };

		auto tc = new Util::TagsCompleter (Ui_.SearchLine_);
		tc->OverrideModel (TagsModel_);

		new Util::ClearLineEditAddon { Core::Instance ().GetProxy (), Ui_.SearchLine_, searchLineButtonMgr };

		auto selector = new Util::CategorySelector ();
		selector->setWindowFlags ({});
		selector->setMinimumHeight (0);
		selector->SetCaption (tr ("Package tags"));
		selector->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Expanding);
		Ui_.SearchLine_->AddSelector (selector);
		Ui_.SearchLayout_->addWidget (selector);
		Ui_.SearchSplitter_->setStretchFactor (0, 2);
		Ui_.SearchSplitter_->setStretchFactor (1, 9);

		connect (&Core::Instance (),
				SIGNAL (tagsUpdated (QStringList)),
				Ui_.SearchLine_,
				SLOT (handleTagsUpdated (QStringList)));
		connect (&Core::Instance (),
				&Core::tagsUpdated,
				selector,
				[selector] (const QStringList& tags) { selector->SetPossibleSelections (tags); });

		connect (&Core::Instance (),
				SIGNAL (tagsUpdated (QStringList)),
				this,
				SLOT (handleTagsUpdated (QStringList)));

		TypeFilter_->setDynamicSortFilter (true);
		TypeFilter_->setSourceModel (Core::Instance ().GetPluginsModel ());
		TypeFilter_->setSortCaseSensitivity (Qt::CaseInsensitive);

		FilterString_->setDynamicSortFilter (true);
		FilterString_->setFilterCaseSensitivity (Qt::CaseInsensitive);
		FilterString_->setSortCaseSensitivity (Qt::CaseInsensitive);
		FilterString_->setSourceModel (TypeFilter_);
		FilterString_->setFilterKeyColumn (PackagesModel::Columns::Name);
		FilterString_->sort (PackagesModel::Columns::Name);

		Ui_.PackagesTree_->setModel (FilterString_);
		Ui_.PackagesTree_->setItemDelegate (new PackagesDelegate (Ui_.PackagesTree_));

		BuildPackageTreeShortcuts ();

		auto setColWidth = [this] (int col, const QString& sample)
		{
			Ui_.PackagesTree_->setColumnWidth (col, fontMetrics ().horizontalAdvance (sample));
		};
		Ui_.PackagesTree_->setColumnWidth (PackagesModel::Columns::Inst, 32);
		Ui_.PackagesTree_->setColumnWidth (PackagesModel::Columns::Upd, 32);
		setColWidth (PackagesModel::Columns::Size, "999 KiB");
		setColWidth (PackagesModel::Columns::Version, "0.1.2.3-r4");
		setColWidth (PackagesModel::Columns::Description,
				"This is a typical package short description for Capcom Fighting Evolution");
		setColWidth (PackagesModel::Columns::Name, "Capcom Fighting Evolution package");

		Ui_.PendingTree_->setModel (Core::Instance ()
				.GetPendingManager ()->GetPendingModel ());

		connect (Ui_.PackagesTree_->selectionModel (),
				SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
				this,
				SLOT (handlePackageSelected (const QModelIndex&)));
		connect (Ui_.SearchLine_,
				SIGNAL (textChanged (const QString&)),
				FilterString_,
				SLOT (setFilterFixedString (const QString&)));

		connect (Core::Instance ().GetPendingManager (),
				SIGNAL (fetchListUpdated (const QList<int>&)),
				this,
				SLOT (handleFetchListUpdated (const QList<int>&)));

		const QStringList& tags = Core::Instance ().GetAllTags ();
		handleTagsUpdated (tags);
		selector->SetPossibleSelections (tags);
		handleFetchListUpdated ({});

		BuildActions ();

		const auto& browsers = Core::Instance ().GetProxy ()->
				GetPluginsManager ()->GetAllCastableTo<IWebBrowser*> ();
		if (browsers.size ())
			Ui_.Browser_->Construct (browsers.at (0));
		Ui_.Browser_->SetNavBarVisible (false);
		Ui_.Browser_->SetEverythingElseVisible (false);
	}

	TabClassInfo LackManTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* LackManTab::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	void LackManTab::Remove ()
	{
		emit removeTab (this);
		deleteLater ();
	}

	QToolBar* LackManTab::GetToolBar () const
	{
		return Toolbar_;
	}

	QByteArray LackManTab::GetTabRecoverData () const
	{
		return "lackmantab";
	}

	QIcon LackManTab::GetTabRecoverIcon () const
	{
		return TC_.Icon_;
	}

	QString LackManTab::GetTabRecoverName () const
	{
		return TC_.VisibleName_;
	}

	void LackManTab::SetFilterTags (const QStringList& tags)
	{
		TypeFilter_->SetFilterMode (TypeFilterProxyModel::FilterMode::All);

		const auto& filter = Core::Instance ().GetProxy ()->GetTagsManager ()->Join (tags);
		FilterString_->setFilterFixedString (filter);

		Ui_.SearchLine_->setTags (tags);
	}

	void LackManTab::SetFilterString (const QString& string)
	{
		TypeFilter_->SetFilterMode (TypeFilterProxyModel::FilterMode::All);
		Ui_.SearchLine_->setText (string);
	}

	void LackManTab::BuildPackageTreeShortcuts ()
	{
		new QShortcut (QString ("K"), this, SLOT (navigateUp ()));
		new QShortcut (QString ("J"), this, SLOT (navigateDown ()));
		new QShortcut (QString ("Space"), this, SLOT (toggleSelected ()));
	}

	void LackManTab::BuildActions ()
	{
		UpdateAll_ = new QAction (tr ("Update all repos"), this);
		UpdateAll_->setProperty ("ActionIcon", "view-refresh");
		UpdateAll_->setShortcut (QString ("Ctrl+U"));
		connect (UpdateAll_,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (updateAllRequested ()));
		ShortcutMgr_->RegisterAction ("updaterepos", UpdateAll_);

		UpgradeAll_ = new QAction (tr ("Upgrade all packages"), this);
		UpgradeAll_->setProperty ("ActionIcon", "system-software-update");
		UpgradeAll_->setShortcut (QString ("Ctrl+Shift+U"));
		connect (UpgradeAll_,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (upgradeAllRequested ()));
		ShortcutMgr_->RegisterAction ("upgradeall", UpgradeAll_);

		Apply_ = new QAction (tr ("Apply"), this);
		Apply_->setProperty ("ActionIcon", "dialog-ok");
		Apply_->setShortcut (QString ("Ctrl+G"));
		Apply_->setEnabled (false);
		connect (Apply_,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (acceptPending ()));
		ShortcutMgr_->RegisterAction ("apply", Apply_);

		Cancel_ = new QAction (tr ("Cancel"), this);
		Cancel_->setProperty ("ActionIcon", "dialog-cancel");
		Cancel_->setEnabled (false);
		connect (Cancel_,
				SIGNAL (triggered ()),
				&Core::Instance (),
				SLOT (cancelPending ()));
		ShortcutMgr_->RegisterAction ("cancel", Cancel_);

		Toolbar_ = new QToolBar ("LackMan");
		Toolbar_->addAction (UpdateAll_);
		Toolbar_->addAction (UpgradeAll_);
		Toolbar_->addSeparator ();
		Toolbar_->addAction (Apply_);
		Toolbar_->addAction (Cancel_);

		const auto pm = Core::Instance ().GetPendingManager ();
		connect (pm,
				SIGNAL (hasPendingActionsChanged (bool)),
				Apply_,
				SLOT (setEnabled (bool)));
		connect (pm,
				SIGNAL (hasPendingActionsChanged (bool)),
				Cancel_,
				SLOT (setEnabled (bool)));

		auto unm = Core::Instance ().GetUpdatesNotificationManager ();
		UpgradeAll_->setEnabled (unm->HasUpgradable ());
		connect (unm,
				SIGNAL (hasUpgradablePackages (bool)),
				UpgradeAll_,
				SLOT (setEnabled (bool)));
	}

	void LackManTab::navigateUp ()
	{
		auto idx = Ui_.PackagesTree_->currentIndex ();
		idx = idx.sibling (idx.row () - 1, 0);
		if (!idx.isValid ())
			return;

		Ui_.PackagesTree_->setCurrentIndex (idx);
	}

	void LackManTab::navigateDown ()
	{
		auto idx = Ui_.PackagesTree_->currentIndex ();
		idx = idx.sibling (idx.row () + 1, 0);
		if (!idx.isValid ())
			return;

		Ui_.PackagesTree_->setCurrentIndex (idx);
	}

	void LackManTab::toggleSelected ()
	{
		auto idx = Ui_.PackagesTree_->currentIndex ();
		if (!idx.isValid ())
			return;

		idx = idx.sibling (idx.row (), PackagesModel::Columns::Upd);
		if (!(idx.flags () & Qt::ItemIsUserCheckable))
			idx = idx.sibling (idx.row (), PackagesModel::Columns::Inst);

		const auto state = idx.data (Qt::CheckStateRole).toInt ();
		Ui_.PackagesTree_->model ()->setData (idx,
				state == Qt::Checked ? Qt::Unchecked : Qt::Checked,
				Qt::CheckStateRole);
	}

	void LackManTab::handlePackageSelected (const QModelIndex& index)
	{
		QString text;
		auto AddText = [&text] (const QStringList& urls)
		{
			for (const auto& url : urls)
				text += "<img src='" + url + "' alt='Image' /><br />";
		};

		AddText (index.data (PackagesModel::PMRThumbnails).toStringList ());

		QString descr = index.data (PackagesModel::PMRLongDescription).toString ();
		if (!descr.contains ("<br"))
			descr.replace ("\n\n", "<br/><br/>");
		text += descr;

		const QStringList& screens = index.data (PackagesModel::PMRScreenshots).toStringList ();
		text += "<hr/>";
		AddText (screens);

		Ui_.Browser_->SetHtml (text);

		if (index.isValid ())
		{
			const auto& nameIndex = index.sibling (index.row (), PackagesModel::Columns::Name);
			const auto& name = nameIndex.data ().toString ();
			Ui_.PackageInfoBox_->setTitle (tr ("Package information: %1").arg (name));
		}
		else
			Ui_.PackageInfoBox_->setTitle (tr ("Package information"));

		QString state;
		if (!index.isValid ()) ;
		else if (!index.data (PackagesModel::PMRInstalled).toBool ())
			state = tr ("not installed");
		else if (index.data (PackagesModel::PMRUpgradable).toBool ())
			state = tr ("installed; upgradable");
		else
			state = tr ("installed");
		Ui_.StateLabel_->setText (state);

		const qint64 size = index.data (PackagesModel::PMRSize).toLongLong ();
		const QString& sizeText = size >= 0 ?
				Util::MakePrettySize (size) :
				tr ("unknown");
		Ui_.SizeLabel_->setText (sizeText);

		const auto& tags = index.data (PackagesModel::PMRTags).toStringList ();
		Ui_.TagsLabel_->setText (tags.join ("; "));
	}

	void LackManTab::handleFetchListUpdated (const QList<int>& ids)
	{
		qint64 sumSize = 0;
		for (const int id : ids)
		{
			if (Core::Instance ().GetListPackageInfo (id).IsInstalled_)
				continue;

			const qint64 size = Core::Instance ().GetStorage ()->GetPackageSize (id);
			if (size > 0)
				sumSize += size;
		}

		if (sumSize > 0)
			Ui_.TotalSizeLabel_->setText (tr ("Total size to be downloaded: %1")
						.arg (Util::MakePrettySize (sumSize)));
		Ui_.TotalSizeLabel_->setVisible (sumSize > 0);
	}

	void LackManTab::handleTagsUpdated (const QStringList& tags)
	{
		TagsModel_->setStringList (tags);
	}

	void LackManTab::toggleInstall ()
	{
		auto idx = sender ()->property ("Index").value<QModelIndex> ();
		idx = idx.sibling (idx.row (), PackagesModel::Inst);

		Ui_.PackagesTree_->model ()->setData (idx,
				idx.data (Qt::CheckStateRole).toInt () == Qt::Checked ?
						Qt::Unchecked :
						Qt::Checked,
				Qt::CheckStateRole);
	}

	void LackManTab::toggleUpgrade ()
	{
		auto idx = sender ()->property ("Index").value<QModelIndex> ();
		idx = idx.sibling (idx.row (), PackagesModel::Upd);

		Ui_.PackagesTree_->model ()->setData (idx,
				idx.data (Qt::CheckStateRole).toInt () == Qt::Checked ?
						Qt::Unchecked :
						Qt::Checked,
				Qt::CheckStateRole);
	}

	void LackManTab::selectAllForInstall ()
	{
		const auto model = Ui_.PackagesTree_->model ();
		for (auto i = 0, rc = model->rowCount (); i < rc; ++i)
			model->setData (model->index (i, PackagesModel::Columns::Inst),
					Qt::Checked, Qt::CheckStateRole);
	}

	void LackManTab::selectNoneForInstall ()
	{
		const auto model = Ui_.PackagesTree_->model ();
		for (auto i = 0, rc = model->rowCount (); i < rc; ++i)
			model->setData (model->index (i, PackagesModel::Columns::Inst),
					Qt::Unchecked, Qt::CheckStateRole);
	}

	void LackManTab::on_PackagesTree__customContextMenuRequested (const QPoint& point)
	{
		QMenu menu;

		const auto& idx = Ui_.PackagesTree_->indexAt (point);

		if (idx.isValid ())
		{
			auto checked = [&idx] (PackagesModel::Columns col) -> bool
			{
				return idx.sibling (idx.row (), col).data (Qt::CheckStateRole).toInt () == Qt::Checked;
			};

			const auto installed = idx.data (PackagesModel::PMRInstalled).toBool ();
			auto installAction = menu.addAction (installed ? tr ("Uninstall") : tr ("Install"),
					this,
					SLOT (toggleInstall ()));
			installAction->setCheckable (true);
			installAction->setChecked (installed ^ checked (PackagesModel::Inst));
			installAction->setProperty ("Index", QVariant::fromValue (idx));

			if (idx.data (PackagesModel::PMRUpgradable).toBool ())
			{
				auto upgradeAction = menu.addAction (tr ("Upgrade"),
						this,
						SLOT (toggleUpgrade ()));
				upgradeAction->setCheckable (true);
				upgradeAction->setChecked (checked (PackagesModel::Upd));
				upgradeAction->setProperty ("Index", QVariant::fromValue (idx));
			}

			menu.addSeparator ();
		}

		menu.addAction (tr ("Mark all for installation"), this, SLOT (selectAllForInstall ()));
		menu.addAction (tr ("Unmark all for installation"), this, SLOT (selectNoneForInstall ()));

		menu.exec (Ui_.PackagesTree_->viewport ()->mapToGlobal (point));
	}

	void LackManTab::on_PackageStatus__currentIndexChanged (int index)
	{
		TypeFilter_->SetFilterMode (static_cast<TypeFilterProxyModel::FilterMode> (index));
	}
}
}
