/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "textboxesdetector.h"
#include <QPagedPaintDevice>
#include <QPaintEngine>
#include <QTextDocument>
#include <interfaces/monocle/ihavetextcontent.h>

namespace LC::Monocle
{
	namespace
	{
		class TextBoxesDetector;

		class TextRecordingPaintEngine : public QPaintEngine
		{
			const QSizeF PageSize_;
			QVector<QVector<TextBox>> Page2Boxes_;
		public:
			explicit TextRecordingPaintEngine (QSizeF pageSize)
			: PageSize_ { pageSize }
			{
			}

			const QVector<QVector<TextBox>>& GetBoxes () const
			{
				return Page2Boxes_;
			}

			bool begin (QPaintDevice*) override { return true; }
			bool end () override { return true; }
			void updateState (const QPaintEngineState&) override {}
			Type type () const override { return Type::User; }

			void drawEllipse (const QRect&) override {}
			void drawEllipse (const QRectF&) override {}
			void drawImage (const QRectF&, const QImage&, const QRectF&, Qt::ImageConversionFlags) override {}
			void drawLines (const QLine*, int) override {}
			void drawLines (const QLineF*, int) override {}
			void drawPath (const QPainterPath&) override {}
			void drawPixmap (const QRectF&, const QPixmap&, const QRectF&) override {}
			void drawPoints (const QPoint*, int) override {}
			void drawPoints (const QPointF*, int) override {}
			void drawPolygon (const QPoint*, int, QPaintEngine::PolygonDrawMode) override {}
			void drawPolygon (const QPointF*, int, QPaintEngine::PolygonDrawMode) override {}
			void drawRects (const QRect*, int) override {}
			void drawRects (const QRectF*, int) override {}
			void drawTiledPixmap (const QRectF&, const QPixmap&, const QPointF&) override {}

			void drawTextItem (const QPointF& srcPos, const QTextItem& text) override
			{
				auto pos = srcPos;
				const auto [page, yPos] = std::div (static_cast<int> (pos.y ()), static_cast<int> (PageSize_.height ()));
				pos.setY (yPos);

				Page2Boxes_.resize (page + 1);
				auto& boxes = Page2Boxes_ [page];

				const QFontMetrics fm { text.font () };
				const auto spaceWidth = fm.horizontalAdvance (' ');

				bool trimLastSpace = false;

				for (const auto& word : text.text ().split (' ', Qt::KeepEmptyParts))
				{
					if (word.isEmpty ())
					{
						trimLastSpace = false;
						continue;
					}

					PageAbsoluteRectBase absRect { fm.boundingRect (word).translated (pos.toPoint ()) };

					boxes << TextBox { .Text_ = word, .Rect_ = absRect.ToPageRelative (PageSize_), .HasSpaceAfter_ = true };
					pos.rx () += fm.horizontalAdvance (word) + spaceWidth;
					trimLastSpace = true;
				}

				if (trimLastSpace)
					boxes.last ().HasSpaceAfter_ = false;
			}
		};

		class TextBoxesDetector : public QPagedPaintDevice
		{
			mutable TextRecordingPaintEngine PaintEngine_;
			const QSizeF PageSize_;
		public:
			explicit TextBoxesDetector (QSizeF pageSize)
			: PaintEngine_ {pageSize }
			{
			}

			bool newPage () override
			{
				return true;
			}

			QPaintEngine* paintEngine () const override
			{
				return &PaintEngine_;
			}

			const QVector<QVector<TextBox>>& GetBoxes () const
			{
				return PaintEngine_.GetBoxes ();
			}
		protected:
			int metric (QPaintDevice::PaintDeviceMetric metric) const override
			{
				constexpr auto dpi = 72;

				switch (metric)
				{
				case QPaintDevice::PdmWidth:
					return static_cast<int> (PageSize_.width ());
				case QPaintDevice::PdmHeight:
					return static_cast<int> (PageSize_.height ());
				case QPaintDevice::PdmWidthMM:
				case QPaintDevice::PdmHeightMM:
					break;
				case QPaintDevice::PdmNumColors:
					return 256;
				case QPaintDevice::PdmDepth:
					return 8;
				case QPaintDevice::PdmDpiX:
				case QPaintDevice::PdmDpiY:
				case QPaintDevice::PdmPhysicalDpiX:
				case QPaintDevice::PdmPhysicalDpiY:
					return dpi;
				case QPaintDevice::PdmDevicePixelRatio:
					return 1;
				case QPaintDevice::PdmDevicePixelRatioScaled:
					return static_cast<int> (devicePixelRatioFScale ());
				}

				return QPagedPaintDevice::metric (metric);
			}
		};
	}

	QVector<QVector<TextBox>> DetectTextBoxes (const QTextDocument& doc)
	{
		TextBoxesDetector detector { doc.pageSize () };
		doc.print (&detector);
		return detector.GetBoxes ();
	}
}
