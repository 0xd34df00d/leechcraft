/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "anntreedelegate.h"
#include <cmath>
#include <QTreeView>
#include <QPainter>
#include <QTextDocument>
#include <QApplication>
#include <QResizeEvent>
#include "annmanager.h"

namespace LC
{
namespace Monocle
{
	const int DocMargin = 4;

	AnnTreeDelegate::AnnTreeDelegate (QTreeView *view, QObject *parent)
	: QStyledItemDelegate { parent }
	, View_ { view }
	{
		View_->viewport ()->installEventFilter (this);
	}

	void AnnTreeDelegate::paint (QPainter *painter,
			const QStyleOptionViewItem& opt, const QModelIndex& index) const
	{
		if (index.data (AnnManager::Role::ItemType) != AnnManager::ItemTypes::AnnItem)
		{
			QStyledItemDelegate::paint (painter, opt, index);
			return;
		}

		painter->save ();
		painter->translate (-View_->indentation (), 0);

		auto option = opt;
		option.rect.setWidth (option.rect.width () + option.decorationSize.width ());

		painter->fillRect (option.rect, QColor { 255, 234, 0 });
		const auto& oldPen = painter->pen ();
		painter->setPen ({ QColor { 255, 213, 0 }, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin });
		painter->drawRect (option.rect);
		painter->setPen (oldPen);

		const auto style = option.widget ? option.widget->style () : QApplication::style ();
		style->drawPrimitive (QStyle::PE_PanelItemViewItem, &option, painter, option.widget);

		painter->save ();
		painter->translate (option.rect.topLeft ());
		GetDoc (index, option.rect.width ())->drawContents (painter);
		painter->restore ();

		painter->restore ();
	}

	QSize AnnTreeDelegate::sizeHint (const QStyleOptionViewItem& opt, const QModelIndex& index) const
	{
		if (index.data (AnnManager::Role::ItemType) != AnnManager::ItemTypes::AnnItem)
			return QStyledItemDelegate::sizeHint (opt, index);

		auto option = opt;
		option.initFrom (View_->viewport ());
		initStyleOption (&option, index);

		auto width = option.rect.width ();

		auto parent = index.parent ();
		while (parent.isValid ())
		{
			width -= View_->indentation ();
			parent = parent.parent ();
		}

		const auto style = option.widget->style ();

		width -= style->pixelMetric (QStyle::PM_LayoutLeftMargin);
		const auto& doc = GetDoc (index, width);
		return
		{
			width,
			static_cast<int> (std::ceil (doc->size ().height ()))
		};
	}

	bool AnnTreeDelegate::eventFilter (QObject *obj, QEvent *event)
	{
		if (event->type () != QEvent::Resize)
			return QStyledItemDelegate::eventFilter (obj, event);

		auto resize = static_cast<QResizeEvent*> (event);
		const auto width = resize->size ().width ();
		if (width == PrevWidth_)
			return QStyledItemDelegate::eventFilter (obj, event);

		PrevWidth_ = width;

		auto model = View_->model ();

		QList<QModelIndex> queue { {} };
		for (int i = 0; i < queue.size (); ++i)
		{
			const auto& idx = queue.at (i);
			for (auto j = 0; j < model->rowCount (idx); ++j)
				queue << model->index (j, 0, idx);
		}

		for (const auto& index : queue)
			if (index.data (AnnManager::Role::ItemType) == AnnManager::ItemTypes::AnnItem)
				emit sizeHintChanged (index);

		return QStyledItemDelegate::eventFilter (obj, event);
	}

	std::shared_ptr<QTextDocument> AnnTreeDelegate::GetDoc (const QModelIndex& index, int width) const
	{
		auto text = std::make_shared<QTextDocument> ();
		text->setTextWidth (width);
		text->setDocumentMargin (DocMargin);
		text->setDefaultStyleSheet ("* { color: black; }");
		text->setHtml (GetText (index));
		return text;
	}

	QString AnnTreeDelegate::GetText (const QModelIndex& index) const
	{
		const auto& ann = index.data (AnnManager::Role::Annotation).value<IAnnotation_ptr> ();

		return QString ("<html><body><strong>%1</strong>: %2<br/>"
				"<strong>%3</strong>: %4<hr/>")
					.arg (tr ("Author"))
					.arg (ann->GetAuthor ())
					.arg (tr ("Date"))
					.arg (QLocale {}.toString (ann->GetDate (), QLocale::ShortFormat)) +
				ann->GetText ().toHtmlEscaped () +
				"</body></html>";
	}
}
}
