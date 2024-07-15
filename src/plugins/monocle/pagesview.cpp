/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "pagesview.h"
#include <QGuiApplication>
#include <QClipboard>
#include <QImageWriter>
#include <QFileDialog>
#include <QMenu>
#include <QMouseEvent>
#include <QPainterPath>
#include <interfaces/core/icoreproxy.h>
#include <util/xpc/stddatafiltermenucreator.h>
#include "interfaces/monocle/idocument.h"
#include "interfaces/monocle/ihavetextcontent.h"
#include "pagegraphicsitem.h"
#include "xmlsettingsmanager.h"

namespace LC::Monocle
{
	void PagesView::SetDocument (IDocument *doc)
	{
		Doc_ = doc;
	}

	void PagesView::SetShowReleaseMenu (bool show)
	{
		ShowReleaseMenu_ = show;
		ShowOnNextRelease_ = false;
	}

	void PagesView::CenterOn (SceneAbsolutePos p)
	{
		centerOn (p.ToPointF ());
	}

	SceneAbsolutePos PagesView::GetCurrentCenter () const
	{
		const auto& rectSize = viewport ()->contentsRect ().size () / 2;
		return SceneAbsolutePos { mapToScene (QPoint { rectSize.width (), rectSize.height () }) };
	}

	SceneAbsolutePos PagesView::GetViewportTrimmedCenter (const QGraphicsItem& item) const
	{
		auto center = item.boundingRect ().bottomRight ();
		center.ry () = std::min (center.y (), static_cast<qreal> (viewport ()->contentsRect ().height ()));
		center /= 2;
		return SceneAbsolutePos { item.mapToScene (center) };
	}

	void PagesView::mouseMoveEvent (QMouseEvent *event)
	{
		if (event->buttons () != Qt::NoButton && ShowReleaseMenu_)
			ShowOnNextRelease_ = true;

		QGraphicsView::mouseMoveEvent (event);
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

		QString GetSelectionText (QGraphicsView& view, IHaveTextContent& ihtc)
		{
			const auto& selectionBound = view.scene ()->selectionArea ().boundingRect ();

			const auto& viewRect = view.mapFromScene (selectionBound).boundingRect ();
			if (viewRect.isEmpty () ||
					viewRect.width () < 4 ||
					viewRect.height () < 4)
			{
				qWarning () << "selection area is empty";
				return {};
			}

			auto item = view.itemAt (viewRect.topLeft ());
			auto pageItem = dynamic_cast<PageGraphicsItem*> (item);
			if (!pageItem)
			{
				qWarning () << "page item is null for" << viewRect.topLeft ();
				return {};
			}

			const auto& docRect = pageItem->MapToDoc (pageItem->mapRectFromScene (selectionBound));
			return ihtc.GetTextContent (pageItem->GetPageNum (), docRect.toRect ());
		}
	}

	void PagesView::mouseReleaseEvent (QMouseEvent *event)
	{
		QGraphicsView::mouseReleaseEvent (event);

		if (!ShowOnNextRelease_ || !Doc_)
			return;

		const auto& selectedImage = GetSelectionImage (*scene ());
		if (selectedImage.isNull ())
			return;

		auto menu = new QMenu { this };
		auto copyAsImage = menu->addAction (tr ("Copy as image"),
				this,
				[selectedImage] { QGuiApplication::clipboard ()->setImage (selectedImage); });
		copyAsImage->setProperty ("ActionIcon", "image-x-generic");

		auto saveAsImage = menu->addAction (tr ("Save as image..."),
				this,
				[selectedImage, this] { SaveImage (selectedImage, this); });
		saveAsImage->setProperty ("ActionIcon", "document-save");

		new Util::StdDataFilterMenuCreator (selectedImage,
				GetProxyHolder ()->GetEntityManager (),
				menu);

		if (const auto ihtc = qobject_cast<IHaveTextContent*> (Doc_->GetQObject ()))
			if (const auto& selText = GetSelectionText (*this, *ihtc);
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

		menu->popup (event->globalPos ());
		menu->setAttribute (Qt::WA_DeleteOnClose);
		menu->show ();

		ShowOnNextRelease_ = false;
	}

	void PagesView::resizeEvent (QResizeEvent *e)
	{
		QGraphicsView::resizeEvent (e);
		emit sizeChanged ();
	}
}
