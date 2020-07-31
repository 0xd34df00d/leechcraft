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
#include <interfaces/core/iiconthememanager.h>
#include "annmanager.h"
#include "anntreedelegate.h"
#include "core.h"

namespace LC
{
namespace Monocle
{
	AnnWidget::AnnWidget (AnnManager *mgr, QWidget *parent)
	: QWidget { parent }
	, Mgr_ { mgr }
	{
		Ui_.setupUi (this);

		const auto sm = Core::Instance ().GetShortcutManager ();
		auto toolbar = new QToolBar;
		auto prevAct = toolbar->addAction (tr ("Previous annotation"),
				mgr, SLOT (selectPrev ()));
		prevAct->setProperty ("ActionIcon", "go-previous");
		sm->RegisterAction ("org.LeechCraft.Monocle.PrevAnn", prevAct);

		auto nextAct = toolbar->addAction (tr ("Next annotation"),
				mgr, SLOT (selectNext ()));
		nextAct->setProperty ("ActionIcon", "go-next");
		sm->RegisterAction ("org.LeechCraft.Monocle.NextAnn", nextAct);

		const auto treeIdx = Ui_.AnnWidgetLayout_->indexOf (Ui_.AnnTree_);
		Ui_.AnnWidgetLayout_->insertWidget (treeIdx, toolbar);

		Ui_.AnnTree_->setItemDelegate (new AnnTreeDelegate { Ui_.AnnTree_, this });
		Ui_.AnnTree_->setModel (Mgr_->GetModel ());

		connect (Mgr_,
				SIGNAL (annotationSelected (QModelIndex)),
				this,
				SLOT (focusOnAnnotation (QModelIndex)));
		connect (Ui_.AnnTree_,
				SIGNAL (activated (QModelIndex)),
				Mgr_,
				SLOT (selectAnnotation (QModelIndex)));
	}

	void AnnWidget::on_AnnTree__customContextMenuRequested (const QPoint& point)
	{
		const auto& idx = Ui_.AnnTree_->indexAt (point);
		if (!idx.isValid () ||
				idx.data (AnnManager::Role::ItemType).toInt () == AnnManager::ItemTypes::PageItem)
			return;

		const auto itm = Core::Instance ().GetProxy ()->GetIconThemeManager ();

		QMenu menu;
		menu.addAction (itm->GetIcon ("edit-copy"),
				tr ("Copy annotation text"),
				[&idx]
				{
					const auto& ann = idx.data (AnnManager::Role::Annotation).value<IAnnotation_ptr> ();
					qApp->clipboard ()->setText (ann->GetText ());
				});
		menu.exec (Ui_.AnnTree_->viewport ()->mapToGlobal (point));
	}

	void AnnWidget::focusOnAnnotation (const QModelIndex& index)
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
}
