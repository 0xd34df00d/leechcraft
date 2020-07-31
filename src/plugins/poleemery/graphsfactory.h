/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <functional>
#include <QObject>
#include <QPair>
#include <QDateTime>

class QwtPlot;
class QwtPlotItem;

namespace LC
{
namespace Poleemery
{
	typedef QPair<QDateTime, QDateTime> DateSpan_t;

	class GraphsFactory
	{
		struct GraphInfo
		{
			QString Name_;
			std::function<QList<QwtPlotItem*> (DateSpan_t)> Creator_;
			std::function<void (QwtPlot*)> Preparer_;
		};
		QList<GraphInfo> Infos_;
	public:
		GraphsFactory ();

		QStringList GetNames () const;

		QList<QwtPlotItem*> CreateItems (int, const DateSpan_t&);
		void PreparePlot (int, QwtPlot*);
	};
}
}
