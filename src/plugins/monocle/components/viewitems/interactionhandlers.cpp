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
#include <util/sll/qtutil.h>
#include <util/sll/util.h>
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
		QGraphicsView::DragMode DragMode_ {};
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

	MovingInteraction::MovingInteraction (PagesView& view, IDocument&, const QVector<PageGraphicsItem*>&)
	: MovingInteraction { view }
	{
	}

	AreaSelectionInteraction::AreaSelectionInteraction (PagesView& view, IDocument& doc, const QVector<PageGraphicsItem*>&)
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
			constexpr auto compression = 100;
			image.save (filename, suffix, compression);
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

		void FillMenuForText (QMenu& menu, const QString& text)
		{
			auto copyAsText = menu.addAction (AreaSelectionInteraction::tr ("Copy as text"),
					[text] { QGuiApplication::clipboard ()->setText (text); });
			copyAsText->setProperty ("ActionIcon", "edit-copy");

			new Util::StdDataFilterMenuCreator (text,
					GetProxyHolder ()->GetEntityManager (),
					&menu);
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
				FillMenuForText (*menu, selText);
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

	struct TextSelectionInteraction::BoxRepr
	{
		TextBox Box_;
		QGraphicsRectItem *Item_;

		bool IsWordPart_ = false;
	};

	TextSelectionInteraction::TextSelectionInteraction (PagesView& view, IDocument& doc, const QVector<PageGraphicsItem*>& pages)
	: InteractionHandler { view, { .DragMode_ = QGraphicsView::NoDrag, .Cursor_ = Qt::IBeamCursor } }
	, IHTC_ { GetIHTC (doc) }
	, Pages_ { pages }
	{
	}

	TextSelectionInteraction::~TextSelectionInteraction () = default;

	namespace
	{
		PageGraphicsItem* GetItemAt (SceneAbsolutePos pos, QGraphicsScene& scene)
		{
			for (const auto item : scene.items (pos.ToPointF ()))
				if (const auto page = dynamic_cast<PageGraphicsItem*> (item))
					return page;

			return nullptr;
		}

		template<typename T, typename F = std::identity>
		auto GetSelectedRange (T& boxes,
				PageRelativePos firstPos, PageRelativePos lastPos,
				const F& proj = {})
		{
			const auto first = std::ranges::find_if (boxes.begin (), boxes.end (),
					[&] (const PageRelativeRect& rect)
					{
						const auto& bottomRight = rect.BottomRight<PageRelativePos> ();
						return bottomRight.BothGeqThan (firstPos) || bottomRight.BothGeqThan (lastPos);
					},
					proj);
			if (first == boxes.end ())
				return std::pair { boxes.end (), boxes.end () };

			const auto contraPos = proj (*first).template BottomRight<PageRelativePos> ().BothGeqThan (firstPos) ?
					lastPos :
					firstPos;

			const auto last = std::ranges::find_if (boxes.rbegin (), std::reverse_iterator { std::next (first) },
					[&] (const PageRelativeRect& rect)
					{
						return rect.TopLeft<PageRelativePos> ().BothLeqThan (contraPos);
					},
					proj).base ();
			return std::pair { first, last };
		}

		auto ToTuple (const TextSelectionInteraction::SelectionCornerInfo& info)
		{
			return std::tuple { info.first->GetPageNum (), info.second.P_.y (), info.second.P_.x () };
		}
	}

	void TextSelectionInteraction::Pressed (QMouseEvent& ev)
	{
		SelectionStart_.reset ();
		EnsureHasSelectionStart (ViewAbsolutePos { ev.localPos () });
	}

	void TextSelectionInteraction::Moved (QMouseEvent& ev)
	{
		if (ev.buttons () == Qt::NoButton)
			return;

		EnsureHasSelectionStart (ViewAbsolutePos { ev.localPos () });

		const auto& selectionEnd = GetPageInfo (ViewAbsolutePos { ev.localPos () });
		if (!SelectionStart_ || !selectionEnd)
			return;

		const auto [selTopLeft, selBottomRight] = std::minmax (*SelectionStart_, *selectionEnd,
				[] (const auto& l, const auto& r) { return ToTuple (l) < ToTuple (r); });

		const auto [startPage, startPos] = selTopLeft;
		const auto [endPage, endPos] = selBottomRight;

		if (startPage == endPage)
			SelectOnPage (*startPage, startPos, endPos);
		else
		{
			SelectOnPage (*startPage, startPos, PageRelativePos { 1, 1 });
			for (int i = startPage->GetPageNum () + 1; i < endPage->GetPageNum (); ++i)
				SelectOnPage (*Pages_ [i], PageRelativePos { 0, 0 }, PageRelativePos { 1, 1 });
			SelectOnPage (*endPage, PageRelativePos { 0, 0 }, endPos);
		}

		for (auto& boxes : std::ranges::subrange (Boxes_.begin (), Boxes_.find (startPage->GetPageNum ())))
			for (auto& box : boxes)
				box.Item_->setVisible (false);
		for (auto& boxes : std::ranges::subrange (std::next (Boxes_.find (endPage->GetPageNum ())), Boxes_.end ()))
			for (auto& box : boxes)
				box.Item_->setVisible (false);
	}

	namespace
	{
		void UndoWordPart (PageGraphicsItem& page, TextSelectionInteraction::BoxRepr& repr)
		{
			if (repr.IsWordPart_)
			{
				repr.IsWordPart_ = false;
				page.SetChildRect (repr.Item_, repr.Box_.Rect_);
			}
		}

		enum class SelectMidWordOpts : std::uint8_t
		{
			None,
			ForceBeginIncluded,
			ForceEndIncluded,
		};

		void SelectMidWord (PageGraphicsItem& page, PageRelativePos startPos, PageRelativePos endPos,
				TextSelectionInteraction::BoxRepr& repr,
				SelectMidWordOpts opts = SelectMidWordOpts::None)
		{
			if (!repr.Box_.Letters_)
				return;

			const auto& firstWordLetters = *repr.Box_.Letters_;

			auto [firstWordBegin, firstWordEnd] = GetSelectedRange (firstWordLetters, startPos, endPos);
			if (opts == SelectMidWordOpts::ForceBeginIncluded)
				firstWordBegin = firstWordLetters.begin ();
			if (opts == SelectMidWordOpts::ForceEndIncluded)
				firstWordEnd = firstWordLetters.end ();

			if (firstWordBegin == firstWordLetters.begin () && firstWordEnd == firstWordLetters.end ())
				UndoWordPart (page, repr);
			else
			{
				repr.IsWordPart_ = true;
				page.SetChildRect (repr.Item_, *firstWordBegin | *(firstWordEnd - 1));
			}
		}

		void SelectMidWords (PageGraphicsItem& page, PageRelativePos startPos, PageRelativePos endPos,
				std::vector<TextSelectionInteraction::BoxRepr>::iterator selBegin,
				std::vector<TextSelectionInteraction::BoxRepr>::iterator selEnd)
		{
			if (selBegin == selEnd)
				return;

			const auto postBegin = selBegin + 1;
			const auto preEnd = selEnd - 1;

			if (postBegin <= preEnd)
				for (auto& repr : std::ranges::subrange (postBegin, preEnd))
					UndoWordPart (page, repr);

			if (selEnd - selBegin == 1)
				SelectMidWord (page, startPos, endPos, *selBegin);
			else
			{
				SelectMidWord (page, startPos, endPos, *selBegin, SelectMidWordOpts::ForceEndIncluded);
				SelectMidWord (page, startPos, endPos, *preEnd, SelectMidWordOpts::ForceBeginIncluded);
			}
		}
	}

	void TextSelectionInteraction::SelectOnPage (PageGraphicsItem& page, PageRelativePos startPos, PageRelativePos endPos)
	{
		auto& boxes = LoadBoxes (page);

		const auto [selBegin, selEnd] = GetSelectedRange (boxes, startPos, endPos,
				[] (const TextSelectionInteraction::BoxRepr& boxRepr) { return boxRepr.Box_.Rect_; });
		for (auto& box : std::ranges::subrange (boxes.begin (), selBegin))
			box.Item_->setVisible (false);
		for (auto& box : std::ranges::subrange (selBegin, selEnd))
			box.Item_->setVisible (true);
		for (auto& box : std::ranges::subrange (selEnd, boxes.end ()))
			box.Item_->setVisible (false);

		SelectMidWords (page, startPos, endPos, selBegin, selEnd);
	}

	namespace
	{
		std::optional<QString> GetNextSpace (NextSpaceKind kind, bool preserveNewlines)
		{
			static const QString space { ' ' };
			static const QString newline { '\n' };
			static const QString newpara = "\n\n"_qs;

			switch (kind)
			{
			case NextSpaceKind::Space:
				return space;
			case NextSpaceKind::NewLine:
				return preserveNewlines ? newline : space;
			case NextSpaceKind::NewPara:
				return newpara;
			case NextSpaceKind::None:
				break;
			}

			return {};
		}

		bool ShouldPreserveNewlines (const QMouseEvent& ev)
		{
			const auto invert = static_cast<bool> (ev.modifiers () & Qt::AltModifier);
			return invert ^ XmlSettingsManager::Instance ().property ("PreserveParaNewlines").toBool ();
		}
	}

	void TextSelectionInteraction::Released (QMouseEvent& ev)
	{
		const auto guard = Util::MakeScopeGuard ([this] { ClearBoxes (); });

		const auto selectedBoxesCount = std::accumulate (Boxes_.begin (), Boxes_.end (), 0,
				[] (int acc, auto&& boxes)
				{
					return acc + std::ranges::count_if (boxes, [] (auto&& box) { return box.Item_->isVisible (); });
				});

		QStringList textBits;
		textBits.reserve (selectedBoxesCount * 2);

		const auto preserveNewlines = ShouldPreserveNewlines (ev);
		for (const auto& boxes : Boxes_)
			for (auto& box : boxes)
				if (box.Item_->isVisible ())
				{
					textBits << box.Box_.Text_;
					if (const auto nextSym = GetNextSpace (box.Box_.NextSpaceKind_, preserveNewlines))
						textBits << *nextSym;
				}

		const auto& text = textBits.join (QStringView {}).trimmed ();
		if (text.isEmpty ())
			return;

		QMenu menu { &View_ };
		FillMenuForText (menu, text);
		menu.exec (ev.globalPos ());
	}

	void TextSelectionInteraction::EnsureHasSelectionStart (ViewAbsolutePos pos)
	{
		if (!SelectionStart_)
			SelectionStart_ = GetPageInfo (pos);
	}

	auto TextSelectionInteraction::LoadBoxes (PageGraphicsItem& item) -> std::vector<BoxRepr>&
	{
		const auto pageNum = item.GetPageNum ();

		auto& infos = Boxes_ [pageNum];
		if (!infos.empty ())
			return infos;

		connect (&item,
				&QObject::destroyed,
				this,
				[this, pageNum] { Boxes_.remove (pageNum); });

		const QPen boxPen { Qt::transparent };
		auto boxBrush = View_.palette ().brush (QPalette::ColorRole::Highlight);

		const auto& boxes = IHTC_.GetTextBoxes (pageNum);
		infos.reserve (boxes.size ());
		for (const auto& box : boxes)
		{
			auto rectItem = new QGraphicsRectItem { &item };
			rectItem->setZValue (1);
			rectItem->setOpacity (0.5);
			rectItem->setPen (boxPen);
			rectItem->setBrush (boxBrush);
			rectItem->setVisible (false);

			item.RegisterChildRect (rectItem,
					box.Rect_,
					[rectItem] (const PageAbsoluteRect& rect) { rectItem->setRect (rect.ToRectF ()); });
			infos.emplace_back (box, rectItem);
		}

		return infos;
	}

	auto TextSelectionInteraction::GetPageInfo (ViewAbsolutePos viewPos) -> std::optional<SelectionCornerInfo>
	{
		const auto& scenePos = viewPos.ToSceneAbsolute (View_);
		const auto item = GetItemAt (scenePos, *View_.scene ());
		if (!item)
			return {};

		return SelectionCornerInfo
		{
			item,
			scenePos.ToPageRelative (*item),
		};
	}

	void TextSelectionInteraction::ClearBoxes ()
	{
		for (const auto& [pageIdx, boxes] : Util::Stlize (Boxes_))
		{
			const auto page = Pages_ [pageIdx];
			for (const auto& box : boxes)
			{
				page->UnregisterChildRect (box.Item_);
				delete box.Item_;
			}
		}
		Boxes_.clear ();
	}
}
