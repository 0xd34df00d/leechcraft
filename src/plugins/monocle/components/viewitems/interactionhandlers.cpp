/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "interactionhandlers.h"
#include <algorithm>
#include <QClipboard>
#include <QGuiApplication>
#include <QImageWriter>
#include <QFileDialog>
#include <QMenu>
#include <QMouseEvent>
#include <interfaces/core/icoreproxy.h>
#include <util/sll/qtutil.h>
#include <util/xpc/stddatafiltermenucreator.h>
#include "interfaces/monocle/idocument.h"
#include "interfaces/monocle/ihavetextcontent.h"
#include "pagegraphicsitem.h"
#include "pagesview.h"
#include "textselectionitem.h"
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
			image.save (filename, suffix.constData (), compression);
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
					&menu,
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

		menu->popup (ev.globalPosition ().toPoint ());
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

	TextSelectionInteraction::TextSelectionInteraction (PagesView& view, IDocument& doc, const QVector<PageGraphicsItem*>& pages)
	: InteractionHandler { view, { .DragMode_ = QGraphicsView::NoDrag, .Cursor_ = Qt::IBeamCursor } }
	, IHTC_ { GetIHTC (doc) }
	, Pages_ { pages.begin (), pages.end () }
	{
	}

	TextSelectionInteraction::~TextSelectionInteraction ()
	{
		ClearSelection ();
	}

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
					[&] (const PageRelativeRectBase& rect)
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
					[&] (const PageRelativeRectBase& rect)
					{
						return rect.TopLeft<PageRelativePos> ().BothLeqThan (contraPos);
					},
					proj).base ();
			return std::pair { first, last };
		}

		auto ToTuple (const PageWithRelativePos& info)
		{
			return std::tuple { info.Page_, info.Pos_.P_.y (), info.Pos_.P_.x () };
		}
	}

	void TextSelectionInteraction::Pressed (QMouseEvent& ev)
	{
		IsSelectionGesture_ = false;
		Granularity_ = Granularity::Letters;

		ClearSelection ();

		SelectionStart_.reset ();
		EnsureHasSelectionStart (ViewAbsolutePos { ev.position () });
	}

	void TextSelectionInteraction::Moved (QMouseEvent& ev)
	{
		if (ev.buttons () != Qt::NoButton)
		{
			IsSelectionGesture_ = true;
			UpdateSelection (ViewAbsolutePos { ev.position () });
		}
	}

	namespace
	{
		struct LetterRange
		{
			qsizetype Begin_;
			qsizetype End_;
		};

		LetterRange WholeBox (const TextBox& box)
		{
			return { 0, box.Text_.size () };
		}

		LetterRange SelectLetters (const TextBox& box, PageRelativePos startPos, PageRelativePos endPos,
				TextSelectionInteraction::Granularity granularity)
		{
			if (granularity == TextSelectionInteraction::Granularity::Words || !box.Letters_)
				return WholeBox (box);

			const auto& letters = *box.Letters_;
			const auto [begin, end] = GetSelectedRange (letters, startPos, endPos);
			if (begin == letters.end ())
				return WholeBox (box);

			return { begin - letters.begin (), end - letters.begin () };
		}

		struct BoxPositionInfo
		{
			bool IsFirst_;
			bool IsLast_;
		};

		BoxPositionInfo GetBoxPositionInfo (const TextSelectionInteraction::Selection& sel, qsizetype boxIdx)
		{
			return { .IsFirst_ = boxIdx == sel.Start_.Box_, .IsLast_ = boxIdx == sel.End_.Box_ };
		}

		LetterRange GetLetters (const TextSelectionInteraction::Selection& sel, BoxPositionInfo boxPos, const TextBox& box)
		{
			return
			{
				.Begin_ = boxPos.IsFirst_ ? sel.Start_.Letter_ : 0,
				.End_ = boxPos.IsLast_ ? sel.End_.Letter_ : box.Text_.size (),
			};
		}

		QVector<PageRelativeRect> GetSelectionRects (const TextSelectionInteraction::PageText& text)
		{
			QVector<PageRelativeRect> rects;
			if (!text.Selection_)
				return rects;

			const auto& sel = *text.Selection_;
			rects.reserve (sel.End_.Box_ - sel.Start_.Box_ + 1);
			for (auto i = sel.Start_.Box_; i <= sel.End_.Box_; ++i)
			{
				const auto& box = text.Boxes_ [i];
				const auto [begin, end] = GetLetters (sel, GetBoxPositionInfo (sel, i), box);
				if (box.Letters_ && (begin > 0 || end < box.Text_.size ()))
					rects << (box.Letters_->at (begin) | box.Letters_->at (end - 1));
				else
					rects << box.Rect_;
			}
			return rects;
		}
	}

	void TextSelectionInteraction::SelectOnPage (int pageNum, PageRelativePos startPos, PageRelativePos endPos)
	{
		auto& text = LoadText (pageNum);
		const auto& boxes = text.Boxes_;

		const auto [selBegin, selEnd] = GetSelectedRange (boxes, startPos, endPos,
				[] (const TextBox& box) { return box.Rect_; });
		if (selBegin == selEnd)
			return;

		const auto selLast = selEnd - 1;
		text.Selection_ = Selection
		{
			.Start_ = { selBegin - boxes.begin (), SelectLetters (*selBegin, startPos, endPos, Granularity_).Begin_ },
			.End_ = { selLast - boxes.begin (), SelectLetters (*selLast, startPos, endPos, Granularity_).End_ },
		};
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
		if (!IsSelectionGesture_)
			return;

		UpdateSelection (ViewAbsolutePos { ev.position () });

		const auto& text = GetSelectedText (ShouldPreserveNewlines (ev));
		if (text.isEmpty ())
		{
			ClearSelection ();
			return;
		}

		const auto menu = new QMenu { &View_ };
		menu->setAttribute (Qt::WA_DeleteOnClose);
		FillMenuForText (*menu, text);
		connect (menu,
				&QMenu::aboutToHide,
				this,
				&TextSelectionInteraction::ClearSelection);
		menu->popup (ev.globalPosition ().toPoint ());
	}

	void TextSelectionInteraction::DoubleClicked (QMouseEvent& ev)
	{
		IsSelectionGesture_ = true;
		Granularity_ = Granularity::Words;
		UpdateSelection (ViewAbsolutePos { ev.position () });
	}

	void TextSelectionInteraction::UpdateSelection (ViewAbsolutePos pos)
	{
		EnsureHasSelectionStart (pos);

		const auto& selectionEnd = GetPageInfo (pos);
		if (!SelectionStart_ || !selectionEnd)
			return;

		const auto [selTopLeft, selBottomRight] = std::minmax (*SelectionStart_, *selectionEnd,
				[] (const auto& l, const auto& r) { return ToTuple (l) < ToTuple (r); });

		const auto [startPage, startPos] = selTopLeft;
		const auto [endPage, endPos] = selBottomRight;

		for (auto& text : Texts_)
			text.Selection_.reset ();

		if (startPage == endPage)
			SelectOnPage (startPage, startPos, endPos);
		else
		{
			SelectOnPage (startPage, startPos, PageRelativePos { 1, 1 });
			for (int i = startPage + 1; i < endPage; ++i)
				SelectOnPage (i, PageRelativePos { 0, 0 }, PageRelativePos { 1, 1 });
			SelectOnPage (endPage, PageRelativePos { 0, 0 }, endPos);
		}

		UpdateOverlays ();
	}

	void TextSelectionInteraction::UpdateOverlays ()
	{
		for (auto&& [page, text] : Texts_.asKeyValueRange ())
		{
			const auto& rects = GetSelectionRects (text);
			if (rects.isEmpty () && !text.Overlay_)
				continue;

			if (!text.Overlay_)
				text.Overlay_ = new TextSelectionItem { *Pages_ [page] };
			text.Overlay_->SetRects (rects);
		}
	}

	void TextSelectionInteraction::EnsureHasSelectionStart (ViewAbsolutePos pos)
	{
		if (!SelectionStart_)
			SelectionStart_ = GetPageInfo (pos);
	}

	auto TextSelectionInteraction::LoadText (int pageNum) -> PageText&
	{
		if (const auto pos = Texts_.find (pageNum); pos != Texts_.end ())
			return *pos;

		return *Texts_.insert (pageNum, { .Boxes_ = IHTC_.GetTextBoxes (pageNum), .Selection_ = {}, .Overlay_ = {} });
	}

	auto TextSelectionInteraction::GetPageInfo (ViewAbsolutePos viewPos) const -> std::optional<PageWithRelativePos>
	{
		const auto& scenePos = viewPos.ToSceneAbsolute (View_);
		const auto item = GetItemAt (scenePos, *View_.scene ());
		if (!item)
			return {};

		return PageWithRelativePos { item->GetPageNum (), scenePos.ToPageRelative (*item) };
	}

	QString TextSelectionInteraction::GetSelectedText (bool preserveNewlines) const
	{
		QStringList textBits;
		for (const auto& [boxes, selection, _] : Texts_)
		{
			if (!selection)
				continue;

			for (auto i = selection->Start_.Box_; i <= selection->End_.Box_; ++i)
			{
				const auto& box = boxes [i];
				const auto [begin, end] = GetLetters (*selection, GetBoxPositionInfo (*selection, i), box);
				textBits << box.Text_.mid (begin, end - begin);
				if (const auto nextSym = GetNextSpace (box.NextSpaceKind_, preserveNewlines))
					textBits << *nextSym;
			}
		}
		return textBits.join (QStringView {}).trimmed ();
	}

	void TextSelectionInteraction::ClearSelection ()
	{
		for (const auto& text : Texts_)
			delete text.Overlay_.data ();
		Texts_.clear ();
	}
}
