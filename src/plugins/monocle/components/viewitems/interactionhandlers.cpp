/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "interactionhandlers.h"
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

	AreaSelectionInteraction::AreaSelectionInteraction (PagesView& view)
	: InteractionHandler { view, { .DragMode_ = QGraphicsView::RubberBandDrag } }
	{
	}

	void AreaSelectionInteraction::Moved (QMouseEvent& ev, IDocument&)
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

	void AreaSelectionInteraction::Released (QMouseEvent& ev, IDocument& doc)
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

		if (const auto ihtc = qobject_cast<IHaveTextContent*> (doc.GetQObject ()))
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
}
