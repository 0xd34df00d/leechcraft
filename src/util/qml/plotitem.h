/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <functional>
#include <optional>
#include <QtGlobal>
#include <QQuickPaintedItem>
#include "qmlconfig.h"

class QwtPlot;

namespace LC::Util
{
	class UTIL_QML_API PlotItem : public QQuickPaintedItem
	{
		Q_OBJECT

		Q_PROPERTY (QList<QPointF> points READ GetPoints WRITE SetPoints NOTIFY pointsChanged)

		Q_PROPERTY (QVariant multipoints READ GetMultipoints WRITE SetMultipoints NOTIFY multipointsChanged)

		Q_PROPERTY (double minXValue READ GetMinXValue WRITE SetMinXValue NOTIFY minXValueChanged)
		Q_PROPERTY (double maxXValue READ GetMaxXValue WRITE SetMaxXValue NOTIFY maxXValueChanged)
		Q_PROPERTY (double minYValue READ GetMinYValue WRITE SetMinYValue NOTIFY minYValueChanged)
		Q_PROPERTY (double maxYValue READ GetMaxYValue WRITE SetMaxYValue NOTIFY maxYValueChanged)

		Q_PROPERTY (bool yGridEnabled READ GetYGridEnabled WRITE SetYGridEnabled NOTIFY yGridChanged)
		Q_PROPERTY (bool yMinorGridEnabled READ GetYMinorGridEnabled WRITE SetYMinorGridEnabled NOTIFY yMinorGridChanged)

		Q_PROPERTY (double alpha READ GetAlpha WRITE SetAlpha NOTIFY alphaChanged)
		Q_PROPERTY (QColor color READ GetColor WRITE SetColor NOTIFY colorChanged)
		Q_PROPERTY (bool leftAxisEnabled READ GetLeftAxisEnabled WRITE SetLeftAxisEnabled NOTIFY leftAxisEnabledChanged)
		Q_PROPERTY (bool bottomAxisEnabled READ GetBottomAxisEnabled WRITE SetBottomAxisEnabled NOTIFY bottomAxisEnabledChanged)
		Q_PROPERTY (QString leftAxisTitle READ GetLeftAxisTitle WRITE SetLeftAxisTitle NOTIFY leftAxisTitleChanged)
		Q_PROPERTY (QString bottomAxisTitle READ GetBottomAxisTitle WRITE SetBottomAxisTitle NOTIFY bottomAxisTitleChanged)

		Q_PROPERTY (QString plotTitle READ GetPlotTitle WRITE SetPlotTitle NOTIFY plotTitleChanged)

		Q_PROPERTY (QColor background READ GetBackground WRITE SetBackground NOTIFY backgroundChanged)
		Q_PROPERTY (QColor textColor READ GetTextColor WRITE SetTextColor NOTIFY textColorChanged)
		Q_PROPERTY (QColor gridLinesColor READ GetGridLinesColor WRITE SetGridLinesColor NOTIFY gridLinesColorChanged)

		Q_PROPERTY (int xExtent READ GetXExtent NOTIFY extentsChanged)
		Q_PROPERTY (int yExtent READ GetYExtent NOTIFY extentsChanged)

		QList<QPointF> Points_;

		struct PointsSet
		{
			QColor Color_;
			std::optional<QColor> BrushColor_;
			QList<QPointF> Points_;
		};
		QList<PointsSet> Multipoints_;

		double MinXValue_ = -1;
		double MaxXValue_ = -1;
		double MinYValue_ = -1;
		double MaxYValue_ = -1;

		bool YGridEnabled_ = false;
		bool YMinorGridEnabled_ = false;

		double Alpha_ = 0.3;

		QColor Color_;

		bool LeftAxisEnabled_ = false;
		bool BottomAxisEnabled_ = false;

		QString LeftAxisTitle_;
		QString BottomAxisTitle_;

		QString PlotTitle_;

		QColor BackgroundColor_;
		QColor TextColor_;
		QColor GridLinesColor_;

		int XExtent_ = 0;
		int YExtent_ = 0;

		std::shared_ptr<QwtPlot> Plot_;
	public:
		explicit PlotItem (QQuickItem* = nullptr);

		QList<QPointF> GetPoints () const;
		void SetPoints (const QList<QPointF>&);

		QVariant GetMultipoints () const;
		void SetMultipoints (const QVariant&);

		double GetMinXValue () const;
		void SetMinXValue (double);
		double GetMaxXValue () const;
		void SetMaxXValue (double);
		double GetMinYValue () const;
		void SetMinYValue (double);
		double GetMaxYValue () const;
		void SetMaxYValue (double);

		bool GetYGridEnabled () const;
		void SetYGridEnabled (bool);
		bool GetYMinorGridEnabled () const;
		void SetYMinorGridEnabled (bool);

		double GetAlpha () const;
		void SetAlpha (double);

		QColor GetColor () const;
		void SetColor (const QColor&);

		bool GetLeftAxisEnabled () const;
		void SetLeftAxisEnabled (bool);
		bool GetBottomAxisEnabled () const;
		void SetBottomAxisEnabled (bool);

		QString GetLeftAxisTitle () const;
		void SetLeftAxisTitle (const QString&);
		QString GetBottomAxisTitle () const;
		void SetBottomAxisTitle (const QString&);

		QString GetPlotTitle () const;
		void SetPlotTitle (const QString&);

		QColor GetBackground () const;
		void SetBackground (const QColor&);
		QColor GetTextColor () const;
		void SetTextColor (const QColor&);
		QColor GetGridLinesColor () const;
		void SetGridLinesColor (const QColor&);

		int GetXExtent () const;
		int GetYExtent () const;

		void paint (QPainter*) override;
	private:
		template<typename T>
		void SetNewValue (T val, T& ourVal, const std::function<void ()>& notifier);

		int CalcXExtent (QwtPlot&) const;
		int CalcYExtent (QwtPlot&) const;
	signals:
		void pointsChanged ();
		void multipointsChanged ();

		void minXValueChanged ();
		void maxXValueChanged ();
		void minYValueChanged ();
		void maxYValueChanged ();

		void yGridChanged ();
		void yMinorGridChanged ();

		void alphaChanged ();

		void colorChanged ();

		void leftAxisEnabledChanged ();
		void bottomAxisEnabledChanged ();

		void leftAxisTitleChanged ();
		void bottomAxisTitleChanged ();

		void plotTitleChanged ();

		void backgroundChanged ();
		void textColorChanged ();
		void gridLinesColorChanged ();

		void extentsChanged ();
	};
}
