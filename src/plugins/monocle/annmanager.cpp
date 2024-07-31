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
#include "components/viewitems/annitem.h"
#include "components/viewitems/pagegraphicsitem.h"

namespace LC::Monocle
{
	AnnManager::AnnManager (LinkExecutionContext& ec, QObject *parent)
	: QObject { parent }
	, AnnModel_ { new QStandardItemModel { this } }
	, ExecutionContext_ { ec }
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
				const auto item = MakeItem (ann, page, ExecutionContext_);
				if (!item)
				{
					qWarning () << "unhandled annotation type" << static_cast<int> (ann->GetAnnotationType ());
					continue;
				}

				Annotations_ << ann;

				Ann2GraphicsItem_ [ann] = item;

				item->SetHandler ([this] (const IAnnotation_ptr& ann) { SelectAnnotation (ann); });

				page->RegisterChildRect (item->GetItem (), ann->GetBoundary (),
						[item] (const PageAbsoluteRect& rect) { item->UpdateRect (rect); });

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

	void AnnManager::SelectAnnotation (const IAnnotation_ptr& ann)
	{
		if (const auto item = Ann2Item_ [ann])
			emit annotationSelected (item->index ());

		const auto newAnn = Annotations_.indexOf (ann);
		if (newAnn == CurrentAnn_)
			return;

		const auto graphicsItem = Ann2GraphicsItem_ [ann];
		if (!graphicsItem)
		{
			const auto type = static_cast<int> (ann->GetAnnotationType ());
			qWarning () << "no item for the annotation" << type << ann->GetText ();
			return;
		}

		emit navigationRequested (*graphicsItem->GetItem ());

		if (graphicsItem->IsSelected ())
			return;

		for (const auto& item : Ann2GraphicsItem_)
			if (item->IsSelected ())
			{
				item->SetSelected (false);
				break;
			}

		graphicsItem->SetSelected (true);
		CurrentAnn_ = newAnn;
	}

	void AnnManager::selectPrev ()
	{
		if (Annotations_.size () < 2)
			return;

		if (--CurrentAnn_ < 0)
			CurrentAnn_ = Annotations_.size () - 1;

		SelectAnnotation (Annotations_.at (CurrentAnn_));
	}

	void AnnManager::selectNext ()
	{
		if (Annotations_.size () < 2)
			return;

		if (CurrentAnn_ == -1 || ++CurrentAnn_ >= Annotations_.size ())
			CurrentAnn_ = 0;

		SelectAnnotation (Annotations_.at (CurrentAnn_));
	}

	void AnnManager::selectAnnotation (const QModelIndex& idx)
	{
		if (idx.data (Role::ItemType).toInt () != ItemTypes::AnnItem)
			return;

		if (const auto& ann = idx.data (Role::Annotation).value<IAnnotation_ptr> ())
			SelectAnnotation (ann);
	}
}
