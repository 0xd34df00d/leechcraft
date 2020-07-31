/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QMap>
#include <QPointF>
#include "structures.h"

namespace LC
{
namespace CpuLoad
{
	struct LoadTypeInfo;

	class CpuLoadProxyObj : public QObject
	{
		Q_OBJECT

		Q_PROPERTY (double ioPercentage READ GetIoPercentage NOTIFY percentagesChanged)
		Q_PROPERTY (double lowPercentage READ GetLowPercentage NOTIFY percentagesChanged)
		Q_PROPERTY (double mediumPercentage READ GetMediumPercentage NOTIFY percentagesChanged)
		Q_PROPERTY (double highPercentage READ GetHighPercentage NOTIFY percentagesChanged)

		Q_PROPERTY (QList<QPointF> ioHist READ GetIoHist NOTIFY histChanged)
		Q_PROPERTY (QList<QPointF> lowHist READ GetLowHist NOTIFY histChanged)
		Q_PROPERTY (QList<QPointF> mediumHist READ GetMediumHist NOTIFY histChanged)
		Q_PROPERTY (QList<QPointF> highHist READ GetHighHist NOTIFY histChanged)

		QMap<LoadPriority, LoadTypeInfo> Infos_;
		QMap<LoadPriority, QList<double>> History_;
	public:
		CpuLoadProxyObj (const QMap<LoadPriority, LoadTypeInfo>&);

		void Set (const QMap<LoadPriority, LoadTypeInfo>&);

		double GetIoPercentage () const;
		double GetLowPercentage () const;
		double GetMediumPercentage () const;
		double GetHighPercentage () const;

		QList<QPointF> GetIoHist () const;
		QList<QPointF> GetLowHist () const;
		QList<QPointF> GetMediumHist () const;
		QList<QPointF> GetHighHist () const;
	private:
		QList<QPointF> GetHist (LoadPriority) const;
	public slots:
		int getMaxX () const;
	signals:
		void percentagesChanged ();
		void histChanged ();
	};
}
}
