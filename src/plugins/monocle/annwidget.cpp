/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "annwidget.h"
#include <QMenu>
#include <QClipboard>
#include <QToolBar>
#include <util/shortcuts/shortcutmanager.h>
#include <util/sll/qtutil.h>
#include <interfaces/core/iiconthememanager.h>
#include "annmanager.h"
#include "anntreedelegate.h"
#include "core.h"

namespace LC::Monocle
{
	AnnWidget::AnnWidget (AnnManager& mgr, QWidget *parent)
	: QWidget { parent }
	, Mgr_ { mgr }
	{
		Ui_.setupUi (this);

		connect (Ui_.AnnTree_,
				&QTreeView::customContextMenuRequested,
				this,
				&AnnWidget::ShowContextMenu);

		const auto sm = Core::Instance ().GetShortcutManager ();
		auto toolbar = new QToolBar;
		auto prevAct = toolbar->addAction (tr ("Previous annotation"),
				&mgr, &AnnManager::selectPrev);
		prevAct->setProperty ("ActionIcon", "go-previous");
		sm->RegisterAction ("org.LeechCraft.Monocle.PrevAnn", prevAct);

		auto nextAct = toolbar->addAction (tr ("Next annotation"),
				&mgr, &AnnManager::selectNext);
		nextAct->setProperty ("ActionIcon", "go-next");
		sm->RegisterAction ("org.LeechCraft.Monocle.NextAnn", nextAct);

		const auto treeIdx = Ui_.AnnWidgetLayout_->indexOf (Ui_.AnnTree_);
		Ui_.AnnWidgetLayout_->insertWidget (treeIdx, toolbar);

		Ui_.AnnTree_->setItemDelegate (new AnnTreeDelegate { Ui_.AnnTree_, this });
		Ui_.AnnTree_->setModel (Mgr_.GetModel ());

		connect (&Mgr_,
				&AnnManager::annotationSelected,
				this,
				&AnnWidget::FocusOnAnnotation);
		connect (Ui_.AnnTree_,
				&QTreeView::activated,
				&Mgr_,
				&AnnManager::selectAnnotation);
	}

	void AnnWidget::ShowContextMenu (QPoint point)
	{
		const auto& idx = Ui_.AnnTree_->indexAt (point);
		if (!idx.isValid () || idx.data (AnnManager::Role::ItemType).toInt () == AnnManager::ItemTypes::PageItem)
			return;

		const auto itm = GetProxyHolder ()->GetIconThemeManager ();

		QMenu menu;
		menu.addAction (itm->GetIcon ("edit-copy"_qs),
				tr ("Copy annotation text"),
				[&idx]
				{
					const auto& ann = idx.data (AnnManager::Role::Annotation).value<IAnnotation_ptr> ();
					qGuiApp->clipboard ()->setText (ann->GetText ());
				});
		menu.exec (Ui_.AnnTree_->viewport ()->mapToGlobal (point));
	}

	void AnnWidget::FocusOnAnnotation (const QModelIndex& index)
	{
		QList<QModelIndex> expandList;
		auto parent = index.parent ();
		while (parent.isValid ())
		{
			expandList.prepend (parent);
			parent = parent.parent ();
		}

		for (const auto& idx : expandList)
			Ui_.AnnTree_->expand (idx);

		Ui_.AnnTree_->setCurrentIndex (index);
		Ui_.AnnTree_->selectionModel ()->select (index, QItemSelectionModel::SelectCurrent);
	}
}
