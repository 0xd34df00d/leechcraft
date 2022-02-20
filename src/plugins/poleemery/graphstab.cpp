/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "graphstab.h"
#include <numeric>
#include <qwt_legend.h>
#include "graphsfactory.h"
#include "core.h"
#include "operationsmanager.h"

namespace LC
{
namespace Poleemery
{
	GraphsTab::GraphsTab (const TabClassInfo& tc, QObject *plugin)
	: TC_ (tc)
	, ParentPlugin_ (plugin)
	{
		Ui_.setupUi (this);

		Ui_.GraphType_->addItems (GraphsFactory ().GetNames ());
		Ui_.GraphType_->setCurrentIndex (-1);

		auto legend = new QwtLegend;
		legend->setDefaultItemMode (QwtLegendData::Clickable);
		Ui_.Plot_->insertLegend (legend, QwtPlot::BottomLegend);

		connect (Ui_.GraphType_,
				SIGNAL (activated (int)),
				this,
				SLOT (updateGraph ()));
		connect (Ui_.From_,
				SIGNAL (dateChanged (QDate)),
				this,
				SLOT (updateGraph ()));
		connect (Ui_.To_,
				SIGNAL (dateChanged (QDate)),
				this,
				SLOT (updateGraph ()));

		connect (Ui_.PredefinedDate_,
				SIGNAL (currentIndexChanged (int)),
				this,
				SLOT (setPredefinedDate (int)));
		setPredefinedDate (0);
		Ui_.To_->setDateTime (QDateTime::currentDateTime ());
	}

	TabClassInfo GraphsTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* GraphsTab::ParentMultiTabs ()
	{
		return ParentPlugin_;
	}

	void GraphsTab::Remove ()
	{
		emit removeTab ();
		deleteLater ();
	}

	QToolBar* GraphsTab::GetToolBar () const
	{
		return 0;
	}

	void GraphsTab::updateGraph ()
	{
		Ui_.Plot_->detachItems ();

		const auto index = Ui_.GraphType_->currentIndex ();
		if (index < 0)
			return;

		const auto& from = Ui_.From_->dateTime ();
		const auto& to = Ui_.To_->dateTime ();

		GraphsFactory f;
		for (const auto& item : f.CreateItems (index, { from, to }))
		{
			item->setRenderHint (QwtPlotItem::RenderAntialiased);
			item->attach (Ui_.Plot_);
		}
		f.PreparePlot (index, Ui_.Plot_);

		Ui_.Plot_->replot ();
	}

	void GraphsTab::setPredefinedDate (int index)
	{
		const auto& now = QDateTime::currentDateTime ();

		QDateTime first;
		switch (index)
		{
		case 0:
			first = now.addDays (-7);
			break;
		case 1:
			first = now.addMonths (-1);
			break;
		default:
		{
			const auto& entries = Core::Instance ().GetOpsManager ()->GetAllEntries ();
			first = std::accumulate (entries.begin (), entries.end (), now,
					[] (const QDateTime& d, EntryBase_ptr e)
						{ return std::min (d, e->Date_); });
			break;
		}
		}

		Ui_.From_->setDateTime (first);
		Ui_.To_->setDateTime (now);

		updateGraph ();
	}
}
}
