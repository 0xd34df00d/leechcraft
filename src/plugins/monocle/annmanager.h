/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include "interfaces/monocle/idocument.h"
#include "interfaces/monocle/iannotation.h"

class QAbstractItemModel;
class QModelIndex;
class QStandardItemModel;
class QStandardItem;

namespace LC::Monocle
{
	class DocumentTab;
	class SmoothScroller;
	class PageGraphicsItem;
	class AnnBaseItem;

	class AnnManager : public QObject
	{
		Q_OBJECT

		DocumentTab& DocTab_;

		SmoothScroller * const Scroller_;

		QStandardItemModel * const AnnModel_;
		QMap<IAnnotation_ptr, QStandardItem*> Ann2Item_;

		QMap<IAnnotation_ptr, AnnBaseItem*> Ann2GraphicsItem_;

		QVector<IAnnotation_ptr> Annotations_;
		int CurrentAnn_ = -1;
	public:
		enum Role : std::uint16_t
		{
			ItemType = Qt::UserRole + 1,
			Annotation
		};

		enum ItemTypes : std::uint8_t
		{
			PageItem,
			AnnHeaderItem,
			AnnItem
		};

		AnnManager (SmoothScroller*, DocumentTab&);

		void HandleDoc (IDocument&, const QVector<PageGraphicsItem*>&);

		QAbstractItemModel* GetModel () const;
	private:
		void EmitSelected (const IAnnotation_ptr&);
		void CenterOn (const IAnnotation_ptr&);
		void SelectAnnotation (const IAnnotation_ptr&);
	public slots:
		void selectPrev ();
		void selectNext ();
		void selectAnnotation (const QModelIndex&);
	signals:
		void annotationSelected (const QModelIndex&);
	};
}
