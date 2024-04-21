/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "export.h"
#include <QFileDialog>
#include <QFuture>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QString>
#include <interfaces/monocle/idocument.h>
#include <interfaces/monocle/isupportpainting.h>

namespace LC::Monocle
{
	void ExportToPdf (IDocument& doc, QWidget& parent)
	{
		if (!doc.GetNumPages ())
			return;

		// TODO check this and disable the action
		const auto paintable = qobject_cast<ISupportPainting*> (doc.GetQObject ());
		if (!paintable)
			return;

		const auto& path = QFileDialog::getSaveFileName (&parent,
				QObject::tr ("Export to PDF"),
				QDir::homePath ());
		if (path.isEmpty ())
			return;

		QPrinter printer;
		printer.setOutputFormat (QPrinter::PdfFormat);
		printer.setOutputFileName (path);
		printer.setPageMargins (QMarginsF { 0, 0, 0, 0 }, QPageLayout::Point);
		printer.setPageSize (QPageSize { doc.GetPageSize (0) });
		printer.setFontEmbeddingEnabled (true);

		QPainter painter { &printer };
		painter.setRenderHint (QPainter::Antialiasing);
		painter.setRenderHint (QPainter::TextAntialiasing);
		painter.setRenderHint (QPainter::SmoothPixmapTransform);

		for (int i = 0, numPages = doc.GetNumPages (); i < numPages; ++i)
		{
			paintable->PaintPage (&painter, i, 1, 1);
			if (i != numPages - 1)
			{
				printer.newPage ();
				painter.translate (0, -doc.GetPageSize (i).height ());
			}
		}
		painter.end ();
	}

	namespace
	{
		auto GetPrintRange (const QPrintDialog& dia, int numPages, int curPage)
		{
			struct Range
			{
				int Start_;
				int End_;
			};

			switch (dia.printRange ())
			{
			case QAbstractPrintDialog::AllPages:
				return Range { .Start_ = 0, .End_ = numPages };
			case QAbstractPrintDialog::Selection:
				return Range {};
			case QAbstractPrintDialog::PageRange:
			{
				const auto& printer = *dia.printer ();
				return Range { .Start_ = printer.fromPage () - 1, .End_ = printer.toPage () };
			}
			case QAbstractPrintDialog::CurrentPage:
				return Range { .Start_ = curPage, .End_ = curPage + 1 };
			}

			return Range {};
		}
	}

	void Print (int curPage, IDocument& doc, QWidget& parent)
	{
		const int numPages = doc.GetNumPages ();

		QPrinter printer { QPrinter::HighResolution };
		printer.setFullPage (true);
		QPrintDialog dia { &printer, &parent };
		dia.setMinMax (1, numPages);
		dia.setOption (QAbstractPrintDialog::PrintToFile);
		if (curPage >= 0)
			dia.setOption (QAbstractPrintDialog::PrintCurrentPage);
		dia.setOption (QAbstractPrintDialog::PrintShowPageSize);
		if (dia.exec () != QDialog::Accepted)
			return;

		const auto& pageRect = printer.pageRect (QPrinter::Point);
		const auto& pageSize = pageRect.size ();
		const auto resScale = printer.resolution () / 72.0;

		const auto& range = GetPrintRange (dia, numPages, curPage);

		const auto isp = qobject_cast<ISupportPainting*> (doc.GetQObject ());

		QPainter painter (&printer);
		painter.setRenderHint (QPainter::Antialiasing);
		painter.setRenderHint (QPainter::TextAntialiasing);
		painter.setRenderHint (QPainter::SmoothPixmapTransform);
		for (int i = range.Start_; i < range.End_; ++i)
		{
			const auto& size = doc.GetPageSize (i);
			const auto scale = std::min (static_cast<double> (pageSize.width ()) / size.width (),
					static_cast<double> (pageSize.height ()) / size.height ());

			if (isp)
				isp->PaintPage (&painter, i, resScale * scale, resScale * scale);
			else
			{
				const auto& img = doc.RenderPage (i, resScale * scale, resScale * scale).result ();
				painter.drawImage (0, 0, img);
			}

			if (i != range.End_ - 1)
				printer.newPage ();
		}
		painter.end ();
	}
}
