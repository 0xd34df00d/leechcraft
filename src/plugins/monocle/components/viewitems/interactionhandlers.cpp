/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "interactionhandlers.h"
#include <ranges>
#include <QClipboard>
#include <QGuiApplication>
#include <QImageWriter>
#include <QFileDialog>
#include <QMenu>
#include <QMouseEvent>
#include <interfaces/core/icoreproxy.h>
#include <util/xpc/stddatafiltermenucreator.h>
#include "interfaces/monocle/idocument.h"
#include "interfaces/monocle/ihavetextcontent.h"
#include "pagegraphicsitem.h"
#include "pagesview.h"
#include "xmlsettingsmanager.h"

namespace LC::Monocle
{
	struct InteractionHandler::ViewConfig
	{
		QGraphicsView::DragMode DragMode_;
		std::optional<Qt::CursorShape> Cursor_ {};
	};

	InteractionHandler::InteractionHandler (PagesView& view, const ViewConfig& cfg)
	: View_ { view }
	{
		if (cfg.Cursor_)
			view.setCursor (*cfg.Cursor_);
		else
			view.unsetCursor ();

		view.setDragMode (cfg.DragMode_);
	}

	MovingInteraction::MovingInteraction (PagesView& view)
	: InteractionHandler { view, { .DragMode_ = QGraphicsView::ScrollHandDrag } }
	{
	}

	MovingInteraction::MovingInteraction (PagesView& view, IDocument&)
	: MovingInteraction { view }
	{
	}

	AreaSelectionInteraction::AreaSelectionInteraction (PagesView& view, IDocument& doc)
	: InteractionHandler { view, { .DragMode_ = QGraphicsView::RubberBandDrag } }
	, Doc_ { doc }
	{
	}

	void AreaSelectionInteraction::Moved (QMouseEvent& ev)
	{
		if (ev.buttons () != Qt::NoButton)
			ShowOnNextRelease_ = true;
	}

	namespace
	{
		QImage GetSelectionImage (QGraphicsScene& scene)
		{
			const auto& bounding = scene.selectionArea ().boundingRect ();
			if (bounding.isEmpty ())
				return {};

			QImage image { bounding.size ().toSize (), QImage::Format_ARGB32 };
			QPainter painter (&image);
			scene.render (&painter, {}, bounding);
			painter.end ();
			return image;
		}

		void SaveImage (const QImage& image, QWidget *parent)
		{
			const auto& previous = XmlSettingsManager::Instance ().Property ("SelectionImageSavePath", QDir::homePath ()).toString ();
			const auto& filename = QFileDialog::getSaveFileName (parent,
					PagesView::tr ("Save selection as"),
					previous,
					PagesView::tr ("PNG images (*.png)"));
			if (filename.isEmpty ())
				return;

			const QFileInfo saveFI (filename);
			XmlSettingsManager::Instance ().setProperty ("SelectionImageSavePath", saveFI.absoluteFilePath ());
			const auto& userSuffix = saveFI.suffix ().toLatin1 ();
			const auto& supported = QImageWriter::supportedImageFormats ();
			const auto suffix = supported.contains (userSuffix) ?
					userSuffix :
					QByteArray { "PNG" };
			image.save (filename, suffix, 100);
		}

		QString GetSelectionText (const QGraphicsScene& scene, IHaveTextContent& ihtc)
		{
			const auto& selectionBound = SceneAbsoluteRect { scene.selectionArea ().boundingRect () };

			QMap<int, QString> pageContents;
			for (const auto item : scene.items (selectionBound.ToRectF ()))
			{
				const auto pageItem = dynamic_cast<PageGraphicsItem*> (item);
				if (!pageItem)
					continue;

				const auto idx = pageItem->GetPageNum ();
				pageContents [idx] = ihtc.GetTextContent (idx, selectionBound.ToPageRelative (*pageItem));
			}
			return QStringList { pageContents.begin (), pageContents.end () }.join ('\n');
		}
	}

	void AreaSelectionInteraction::Released (QMouseEvent& ev)
	{
		if (!ShowOnNextRelease_)
			return;
		ShowOnNextRelease_ = false;

		const auto& selectedImage = GetSelectionImage (*View_.scene ());
		if (selectedImage.isNull ())
			return;

		auto menu = new QMenu { &View_ };
		auto copyAsImage = menu->addAction (tr ("Copy as image"),
				this,
				[selectedImage] { QGuiApplication::clipboard ()->setImage (selectedImage); });
		copyAsImage->setProperty ("ActionIcon", "image-x-generic");

		auto saveAsImage = menu->addAction (tr ("Save as image..."),
				this,
				[selectedImage, this] { SaveImage (selectedImage, &View_); });
		saveAsImage->setProperty ("ActionIcon", "document-save");

		new Util::StdDataFilterMenuCreator (selectedImage,
				GetProxyHolder ()->GetEntityManager (),
				menu);

		if (const auto ihtc = qobject_cast<IHaveTextContent*> (Doc_.GetQObject ()))
			if (const auto& selText = GetSelectionText (*View_.scene (), *ihtc);
				!selText.isEmpty ())
			{
				menu->addSeparator ();

				auto copyAsText = menu->addAction (tr ("Copy as text"),
						this,
						[selText] { QGuiApplication::clipboard ()->setText (selText); });
				copyAsText->setProperty ("ActionIcon", "edit-copy");

				new Util::StdDataFilterMenuCreator (selText,
						GetProxyHolder ()->GetEntityManager (),
						menu);
			}

		menu->popup (ev.globalPos ());
		menu->setAttribute (Qt::WA_DeleteOnClose);
		menu->show ();
	}

	namespace
	{
		IHaveTextContent& GetIHTC (IDocument& doc)
		{
			const auto ptr = qobject_cast<IHaveTextContent*> (doc.GetQObject ());
			if (!ptr)
				throw std::runtime_error { "the document is not an IHaveTextContent" };

			return *ptr;
		}
	}

	struct TextSelectionInteraction::BoxInfo
	{
		TextBox Box_;
		QGraphicsRectItem *Item_;
		bool IsSelected_ = false;
	};

	TextSelectionInteraction::TextSelectionInteraction (PagesView& view, IDocument& doc)
	: InteractionHandler { view, { .DragMode_ = QGraphicsView::NoDrag, .Cursor_ = Qt::IBeamCursor } }
	, IHTC_ { GetIHTC (doc) }
	{
	}

	TextSelectionInteraction::~TextSelectionInteraction () = default;

	namespace
	{
		PageGraphicsItem* GetItemAt (QPointF pos, QGraphicsScene& scene)
		{
			for (const auto item : scene.items (pos))
				if (const auto page = dynamic_cast<PageGraphicsItem*> (item))
					return page;

			return nullptr;
		}

		auto GetSelectedRange (const std::vector<TextSelectionInteraction::BoxInfo>& boxes,
				PageRelativePos firstPos, PageRelativePos lastPos)
		{
			const auto begin = std::find_if (boxes.begin (), boxes.end (),
					[&] (const TextSelectionInteraction::BoxInfo& boxInfo)
					{
						return boxInfo.Box_.Rect_.BottomRight<PageRelativePos> ().BothGeqThan (firstPos);
					});
			if (begin == boxes.end ())
				return std::pair { boxes.end (), boxes.end () };

			auto lastSelected = std::find_if (boxes.rbegin (), std::reverse_iterator { std::next (begin) },
					[&] (const TextSelectionInteraction::BoxInfo& boxInfo)
					{
						return boxInfo.Box_.Rect_.TopLeft<PageRelativePos> ().BothLeqThan (lastPos);
					});

			return std::pair { begin, lastSelected.base () };
		}
	}

	void TextSelectionInteraction::Pressed (QMouseEvent& ev)
	{
		SelectionStart_.reset ();
		EnsureHasSelectionStart (ev.localPos ());
	}

	void TextSelectionInteraction::Moved (QMouseEvent& ev)
	{
		if (ev.buttons () == Qt::NoButton)
			return;

		EnsureHasSelectionStart (ev.localPos ());

		const auto& selectinEndInfo = GetPageInfo (ev.localPos ());
		if (!selectinEndInfo)
			return;

		const auto [page, endPos] = *selectinEndInfo;

		auto& boxes = LoadBoxes (*page);

		const auto [selBegin, selEnd] = GetSelectedRange (boxes, SelectionStart_->second, endPos);
		for (const auto& box : std::ranges::subrange (boxes.begin (), selBegin))
			box.Item_->setBrush ({});
		for (const auto& box : std::ranges::subrange (selBegin, selEnd))
			box.Item_->setBrush (Qt::black);
		for (const auto& box : std::ranges::subrange (selEnd, boxes.end ()))
			box.Item_->setBrush ({});
	}

	void TextSelectionInteraction::Released (QMouseEvent&)
	{
	}

	void TextSelectionInteraction::EnsureHasSelectionStart (QPointF pos)
	{
		if (!SelectionStart_)
			SelectionStart_ = GetPageInfo (pos);
	}

	auto TextSelectionInteraction::LoadBoxes (PageGraphicsItem& item) -> std::vector<BoxInfo>&
	{
		const auto pageNum = item.GetPageNum ();

		auto& infos = Boxes_ [pageNum];
		if (!infos.empty ())
			return infos;

		connect (&item,
				&QObject::destroyed,
				this,
				[this, pageNum] { Boxes_.remove (pageNum); });

		const auto& boxes = IHTC_.GetTextBoxes (pageNum);
		infos.reserve (boxes.size ());
		for (const auto& box : boxes)
		{
			auto rectItem = new QGraphicsRectItem { &item };
			rectItem->setPen (QPen { Qt::black });
			rectItem->setVisible (true);
			rectItem->setZValue (1);

			item.RegisterChildRect (rectItem,
					box.Rect_,
					[rectItem] (const PageAbsoluteRect& rect) { rectItem->setRect (rect.ToRectF ()); });
			infos.emplace_back (box, rectItem);
		}

		return infos;
	}

	auto TextSelectionInteraction::GetPageInfo (QPointF pos) -> std::optional<SelectionCornerInfo>
	{
		const auto item = GetItemAt (pos, *View_.scene ());
		if (!item)
			return {};

		return SelectionCornerInfo
		{
			item,
			SceneAbsolutePos { pos }.ToPageRelative (*item),
		};
	}
}
