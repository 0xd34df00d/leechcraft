/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "cpuloadproxyobj.h"
#include <QtDebug>
#include "structures.h"

namespace LC
{
namespace CpuLoad
{
	const auto HistCount = 500;
	const qreal PointsPerPixel = 2.0;

	CpuLoadProxyObj::CpuLoadProxyObj (const QMap<LoadPriority, LoadTypeInfo>& infos)
	{
		Set (infos);
	}

	void CpuLoadProxyObj::Set (const QMap<LoadPriority, LoadTypeInfo>& infos)
	{
		Infos_ = infos;
		emit percentagesChanged ();

		for (auto i = infos.begin (), end = infos.end (); i != end; ++i)
		{
			auto& arr = History_ [i.key ()];
			arr << i.value ().LoadPercentage_ * 100;
			if (arr.size () > HistCount)
				arr.removeFirst ();
		}
		emit histChanged ();
	}

	double CpuLoadProxyObj::GetIoPercentage () const
	{
		return Infos_ [LoadPriority::IO].LoadPercentage_;
	}

	double CpuLoadProxyObj::GetLowPercentage () const
	{
		return Infos_ [LoadPriority::Low].LoadPercentage_;
	}

	double CpuLoadProxyObj::GetMediumPercentage () const
	{
		return Infos_ [LoadPriority::Medium].LoadPercentage_;
	}

	double CpuLoadProxyObj::GetHighPercentage () const
	{
		return Infos_ [LoadPriority::High].LoadPercentage_;
	}

	QList<QPointF> CpuLoadProxyObj::GetIoHist () const
	{
		return GetHist (LoadPriority::IO);
	}

	QList<QPointF> CpuLoadProxyObj::GetLowHist () const
	{
		return GetHist (LoadPriority::Low);
	}

	QList<QPointF> CpuLoadProxyObj::GetMediumHist () const
	{
		return GetHist (LoadPriority::Medium);
	}

	QList<QPointF> CpuLoadProxyObj::GetHighHist () const
	{
		return GetHist (LoadPriority::High);
	}

	QList<QPointF> CpuLoadProxyObj::GetHist (LoadPriority key) const
	{
		QList<QPointF> result;
		int i = 0;
		for (const auto pt : History_ [key])
			result.push_back ({ i++ / PointsPerPixel, pt });
		return result;
	}

	int CpuLoadProxyObj::getMaxX () const
	{
		return HistCount / PointsPerPixel;
	}
}
}
