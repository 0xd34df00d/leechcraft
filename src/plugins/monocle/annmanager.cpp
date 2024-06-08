/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "annmanager.h"
#include <QStandardItemModel>
#include <QtDebug>
#include "interfaces/monocle/isupportannotations.h"
#include "interfaces/monocle/iannotation.h"
#include "annitem.h"
#include "documenttab.h"
#include "pagegraphicsitem.h"
#include "smoothscroller.h"

namespace LC::Monocle
{
	AnnManager::AnnManager (SmoothScroller *scroller, DocumentTab& docTab)
	: QObject { &docTab }
	, DocTab_ { docTab }
	, Scroller_ { scroller }
	, AnnModel_ { new QStandardItemModel { this } }
	{
	}

	void AnnManager::HandleDoc (IDocument& doc, const QVector<PageGraphicsItem*>& pages)
	{
		if (const auto rc = AnnModel_->rowCount ())
			AnnModel_->removeRows (0, rc);

		Ann2Item_.clear ();
		Ann2GraphicsItem_.clear ();
		Annotations_.clear ();
		CurrentAnn_ = -1;

		const auto isa = qobject_cast<ISupportAnnotations*> (doc.GetQObject ());
		if (!isa)
			return;

		for (auto page : pages)
		{
			QStandardItem *pageItem = nullptr;
			auto createItem = [&pageItem, page, this]
			{
				if (pageItem)
					return;

				pageItem = new QStandardItem { tr ("Page %1").arg (page->GetPageNum () + 1) };
				pageItem->setData (ItemTypes::PageItem, Role::ItemType);
				pageItem->setEditable (false);
				AnnModel_->appendRow (pageItem);
			};

			for (const auto& ann : isa->GetAnnotations (page->GetPageNum ()))
			{
				const auto item = MakeItem (ann, page, DocTab_);
				if (!item)
				{
					qWarning () << Q_FUNC_INFO
							<< "unhandled annotation type"
							<< static_cast<int> (ann->GetAnnotationType ());
					continue;
				}

				Annotations_ << ann;

				Ann2GraphicsItem_ [ann] = item;

				item->SetHandler ([this] (const IAnnotation_ptr& ann)
						{
							EmitSelected (ann);
							SelectAnnotation (ann);
						});

				page->RegisterChildRect (item->GetItem (), ann->GetBoundary (),
						[item] (const QRectF& rect) { item->UpdateRect (rect); });

				auto annItem = new QStandardItem (ann->GetText ());
				annItem->setToolTip (ann->GetText ());
				annItem->setEditable (false);
				annItem->setData (QVariant::fromValue (ann), Role::Annotation);
				annItem->setData (ItemTypes::AnnHeaderItem, Role::ItemType);

				auto subItem = new QStandardItem (ann->GetText ());
				subItem->setToolTip (ann->GetText ());
				subItem->setEditable (false);
				subItem->setData (QVariant::fromValue (ann), Role::Annotation);
				subItem->setData (ItemTypes::AnnItem, Role::ItemType);

				Ann2Item_ [ann] = subItem;

				annItem->appendRow (subItem);
				createItem ();
				pageItem->appendRow (annItem);
			}
		}
	}

	QAbstractItemModel* AnnManager::GetModel () const
	{
		return AnnModel_;
	}

	void AnnManager::EmitSelected (const IAnnotation_ptr& ann)
	{
		if (const auto item = Ann2Item_ [ann])
			emit annotationSelected (item->index ());
	}

	void AnnManager::CenterOn (const IAnnotation_ptr& ann)
	{
		const auto item = Ann2GraphicsItem_.value (ann);
		if (!item)
			return;

		const auto graphicsItem = item->GetItem ();
		const auto& mapped = graphicsItem->scenePos ();
		Scroller_->SmoothCenterOn (mapped.x (), mapped.y ());
	}

	void AnnManager::SelectAnnotation (const IAnnotation_ptr& ann)
	{
		const auto modelItem = Ann2Item_ [ann];
		if (!modelItem)
			return;

		const auto graphicsItem = Ann2GraphicsItem_ [ann];
		if (graphicsItem->IsSelected ())
			return;

		for (const auto& item : Ann2GraphicsItem_)
			if (item->IsSelected ())
			{
				item->SetSelected (false);
				break;
			}

		graphicsItem->SetSelected (true);
		CurrentAnn_ = Annotations_.indexOf (ann);
	}

	void AnnManager::selectPrev ()
	{
		if (Annotations_.size () < 2)
			return;

		if (--CurrentAnn_ < 0)
			CurrentAnn_ = Annotations_.size () - 1;

		const auto& ann = Annotations_.at (CurrentAnn_);

		EmitSelected (ann);
		CenterOn (ann);
		SelectAnnotation (ann);
	}

	void AnnManager::selectNext ()
	{
		if (Annotations_.size () < 2)
			return;

		if (CurrentAnn_ == -1 || ++CurrentAnn_ >= Annotations_.size ())
			CurrentAnn_ = 0;

		const auto& ann = Annotations_.at (CurrentAnn_);

		EmitSelected (ann);
		CenterOn (ann);
		SelectAnnotation (ann);
	}

	void AnnManager::selectAnnotation (const QModelIndex& idx)
	{
		if (idx.data (Role::ItemType).toInt () != ItemTypes::AnnItem)
			return;

		const auto& ann = idx.data (Role::Annotation).value<IAnnotation_ptr> ();
		if (!ann)
			return;

		CenterOn (ann);

		if (Annotations_.indexOf (ann) == CurrentAnn_)
			return;

		SelectAnnotation (ann);
	}
}
