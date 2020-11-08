/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "plotitem.h"
#include <cmath>
#include <limits>
#include <vector>
#include <memory>
#include <QStyleOption>
#include <QColor>
#include <util/sll/prelude.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_grid.h>
#include <qwt_scale_draw.h>
#include <qwt_text_label.h>
#include <qwt_plot_canvas.h>

Q_DECLARE_METATYPE (QList<QPointF>)

namespace LC::Util
{
	PlotItem::PlotItem (QQuickItem *parent)
	: QQuickPaintedItem { parent }
	, Color_ { "#FF4B10" }
	{
		setFlag (ItemHasContents, true);
	}

	QList<QPointF> PlotItem::GetPoints () const
	{
		return Points_;
	}

	void PlotItem::SetPoints (const QList<QPointF>& pts)
	{
		if (pts == Points_)
			return;

		Points_ = pts;
		emit pointsChanged ();
		update ();
	}

	QVariant PlotItem::GetMultipoints () const
	{
		QVariantList result;
		for (const auto& set : Multipoints_)
		{
			QVariantMap map
			{
				{ "color", QVariant::fromValue (set.Color_) },
				{ "points", QVariant::fromValue (set.Points_) }
			};

			if (set.BrushColor_)
				map ["brushColor"] = *set.BrushColor_;

			result << map;
		}
		return result;
	}

	namespace
	{
		struct UnsupportedType
		{
			const char * const Field_;
			const QVariant Value_;
		};
	}

	void PlotItem::SetMultipoints (const QVariant& variant)
	{
		Multipoints_.clear ();

		try
		{
			for (const auto& set : variant.toList ())
			{
				const auto& map = set.toMap ();

				const auto& colorVar = map ["color"];
				const auto& color = colorVar.toString ();
				if (color.isEmpty ())
					throw UnsupportedType { "`color` expected to be a QString", colorVar };

				const auto& pointsVar = map ["points"];
				QList<QPointF> points;
				if (pointsVar.canConvert<QList<QPointF>> ())
					points = pointsVar.value<QList<QPointF>> ();
				else if (pointsVar.canConvert<QVariantList> ())
					points = Util::Map (pointsVar.toList (),
							[] (const QVariant& var)
							{
								if (var.canConvert<QPointF> ())
									return var.toPointF ();
								else
									throw UnsupportedType { "point element expected to be a QPointF", var };
							});

				std::optional<QColor> brushColor;
				if (const auto& brushVar = map ["brushColor"];
					!brushVar.isNull ())
				{
					if (!brushVar.canConvert<QString> ())
						throw UnsupportedType { "`brush` expected to be a QString", brushVar };
					brushColor = QColor { brushVar.toString () };
				}

				Multipoints_.append ({ color, brushColor, points });
			}
		}
		catch (const UnsupportedType& ty)
		{
			qCritical () << Q_FUNC_INFO
					<< "invalid multipoints map: "
					<< ty.Field_
					<< " but got instead"
					<< ty.Value_;
			return;
		}

		update ();
	}

	double PlotItem::GetMinXValue () const
	{
		return MinXValue_;
	}

	void PlotItem::SetMinXValue (double val)
	{
		SetNewValue (val, MinXValue_, [this] { emit minXValueChanged (); });
	}

	double PlotItem::GetMaxXValue () const
	{
		return MaxXValue_;
	}

	void PlotItem::SetMaxXValue (double val)
	{
		SetNewValue (val, MaxXValue_, [this] { emit maxXValueChanged (); });
	}

	double PlotItem::GetMinYValue () const
	{
		return MinYValue_;
	}

	void PlotItem::SetMinYValue (double val)
	{
		SetNewValue (val, MinYValue_, [this] { emit minYValueChanged (); });
	}

	double PlotItem::GetMaxYValue () const
	{
		return MaxYValue_;
	}

	void PlotItem::SetMaxYValue (double val)
	{
		SetNewValue (val, MaxYValue_, [this] { emit maxYValueChanged (); });
	}

	bool PlotItem::GetYGridEnabled () const
	{
		return YGridEnabled_;
	}

	void PlotItem::SetYGridEnabled (bool val)
	{
		SetNewValue (val, YGridEnabled_, [this] { emit yGridChanged (); });
	}

	bool PlotItem::GetYMinorGridEnabled () const
	{
		return YMinorGridEnabled_;
	}

	void PlotItem::SetYMinorGridEnabled (bool val)
	{
		SetNewValue (val, YMinorGridEnabled_, [this] { emit yMinorGridChanged (); });
	}

	double PlotItem::GetAlpha () const
	{
		return Alpha_;
	}

	void PlotItem::SetAlpha (double a)
	{
		Alpha_ = a;
		emit alphaChanged ();
	}

	QColor PlotItem::GetColor () const
	{
		return Color_;
	}

	void PlotItem::SetColor (const QColor& color)
	{
		SetNewValue (color, Color_, [this] { emit colorChanged (); });
	}

	bool PlotItem::GetLeftAxisEnabled () const
	{
		return LeftAxisEnabled_;
	}

	void PlotItem::SetLeftAxisEnabled (bool enabled)
	{
		SetNewValue (enabled, LeftAxisEnabled_, [this] { emit leftAxisEnabledChanged (); });
	}

	bool PlotItem::GetBottomAxisEnabled () const
	{
		return BottomAxisEnabled_;
	}

	void PlotItem::SetBottomAxisEnabled (bool enabled)
	{
		SetNewValue (enabled, BottomAxisEnabled_, [this] { emit bottomAxisEnabledChanged (); });
	}

	QString PlotItem::GetLeftAxisTitle () const
	{
		return LeftAxisTitle_;
	}

	void PlotItem::SetLeftAxisTitle (const QString& title)
	{
		SetNewValue (title, LeftAxisTitle_, [this] { emit leftAxisTitleChanged (); });
	}

	QString PlotItem::GetBottomAxisTitle () const
	{
		return BottomAxisTitle_;
	}

	void PlotItem::SetBottomAxisTitle (const QString& title)
	{
		SetNewValue (title, BottomAxisTitle_, [this] { emit bottomAxisTitleChanged (); });
	}

	QString PlotItem::GetPlotTitle () const
	{
		return PlotTitle_;
	}

	void PlotItem::SetPlotTitle (const QString& title)
	{
		SetNewValue (title, PlotTitle_, [this] { emit plotTitleChanged (); });
	}

	QColor PlotItem::GetBackground () const
	{
		return BackgroundColor_;
	}

	void PlotItem::SetBackground (const QColor& bg)
	{
		SetNewValue (bg, BackgroundColor_, [this] { emit backgroundChanged (); });
	}

	QColor PlotItem::GetTextColor () const
	{
		return TextColor_;
	}

	void PlotItem::SetTextColor (const QColor& color)
	{
		SetNewValue (color, TextColor_, [this] { emit textColorChanged (); });
	}

	QColor PlotItem::GetGridLinesColor () const
	{
		return GridLinesColor_;
	}

	void PlotItem::SetGridLinesColor (const QColor& color)
	{
		SetNewValue (color, GridLinesColor_, [this] { emit gridLinesColorChanged (); });
	}

	int PlotItem::GetXExtent () const
	{
		return XExtent_;
	}

	int PlotItem::GetYExtent () const
	{
		return YExtent_;
	}

	void PlotItem::paint (QPainter *painter)
	{
		const auto& rect = contentsBoundingRect ().toRect ();

		if (!Plot_)
		{
			Plot_ = std::make_shared<QwtPlot> ();
			Plot_->setFrameShape (QFrame::NoFrame);
			Plot_->setFrameShadow (QFrame::Plain);
			Plot_->setLineWidth (0);
			Plot_->setMidLineWidth (0);

			if (const auto canvas = qobject_cast<QwtPlotCanvas*> (Plot_->canvas ()))
				canvas->setBorderRadius (0);
		}

		auto& plot = *Plot_;
		plot.enableAxis (QwtPlot::yLeft, LeftAxisEnabled_);
		plot.enableAxis (QwtPlot::xBottom, BottomAxisEnabled_);
		plot.setAxisTitle (QwtPlot::yLeft, LeftAxisTitle_);
		plot.setAxisTitle (QwtPlot::xBottom, BottomAxisTitle_);

		if (plot.size () != rect.size ())
			plot.resize (rect.size ());

		auto setPaletteColor = [&plot] (const QColor& color, QPalette::ColorRole role)
		{
			if (!color.isValid ())
				return;

			auto pal = plot.palette ();
			pal.setColor (role, { color });
			plot.setPalette (pal);
		};

		setPaletteColor (BackgroundColor_, QPalette::Window);
		setPaletteColor (TextColor_, QPalette::WindowText);
		setPaletteColor (TextColor_, QPalette::Text);

		if (!PlotTitle_.isEmpty ())
			plot.setTitle (QwtText { PlotTitle_ });

		if (MinYValue_ < MaxYValue_)
		{
			plot.setAxisAutoScale (QwtPlot::yLeft, false);
			plot.setAxisScale (QwtPlot::yLeft, MinYValue_, MaxYValue_);
		}
		plot.setAutoFillBackground (false);
		plot.setCanvasBackground (Qt::transparent);

		if (YGridEnabled_)
		{
			auto grid = new QwtPlotGrid;
			grid->enableYMin (YMinorGridEnabled_);
			grid->enableX (false);
			grid->setMajorPen (QPen (GridLinesColor_, 1, Qt::SolidLine));
			grid->setMinorPen (QPen (GridLinesColor_, 1, Qt::DashLine));
			grid->attach (&plot);
		}

		auto items = Multipoints_;
		if (items.isEmpty ())
			items.push_back ({ Color_, {}, Points_ });

		if (MinXValue_ < MaxXValue_)
			plot.setAxisScale (QwtPlot::xBottom, MinXValue_, MaxXValue_);
		else if (const auto ptsCount = items.first ().Points_.size ())
			plot.setAxisScale (QwtPlot::xBottom, 0, ptsCount - 1);

		std::vector<std::unique_ptr<QwtPlotCurve>> curves;
		for (const auto& item : items)
		{
			curves.emplace_back (new QwtPlotCurve);
			const auto curve = curves.back ().get ();

			curve->setPen (QPen (item.Color_));

			if (item.BrushColor_)
				curve->setBrush (*item.BrushColor_);
			else
			{
				auto brushColor = item.Color_;
				brushColor.setAlphaF (Alpha_);
				curve->setBrush (brushColor);
			}

			curve->setRenderHint (QwtPlotItem::RenderAntialiased);
			curve->attach (&plot);

			curve->setSamples (item.Points_.toVector ());
		}

		plot.replot ();

		QwtPlotRenderer {}.render (&plot, painter, rect);

		const auto xExtent = CalcXExtent (plot);
		const auto yExtent = CalcYExtent (plot);
		if (xExtent != XExtent_ || yExtent != YExtent_)
		{
			XExtent_ = xExtent;
			YExtent_ = yExtent;
			emit extentsChanged ();
		}
	}

	template<typename T, typename Notifier>
	void PlotItem::SetNewValue (T val, T& ourVal, Notifier&& notifier)
	{
		if (val == ourVal)
			return;

		ourVal = val;
		notifier ();
		update ();
	}

	int PlotItem::CalcXExtent (QwtPlot& plot) const
	{
		int result = 0;
		if (LeftAxisEnabled_)
			result += plot.axisScaleDraw (QwtPlot::yLeft)->extent (plot.axisFont (QwtPlot::yLeft));
		return result;
	}

	int PlotItem::CalcYExtent (QwtPlot& plot) const
	{
		int result = 0;
		if (BottomAxisEnabled_)
			result += plot.axisScaleDraw (QwtPlot::xBottom)->extent (plot.axisFont (QwtPlot::xBottom));
		if (!PlotTitle_.isEmpty ())
			result += plot.titleLabel ()->sizeHint ().height ();
		return result;
	}
}
