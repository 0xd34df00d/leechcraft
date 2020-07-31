/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QSize>
#include <QList>
#include <QImage>
#include <QMap>
#include <QElapsedTimer>
#include <QtDebug>
#include <util/sll/qtutil.h>

inline bool operator< (const QSize& s1, const QSize& s2)
{
	return std::make_pair (s1.width (), s1.height ()) < std::make_pair (s2.width (), s2.height ());
}

namespace LC
{
namespace Poshuku
{
namespace DCAC
{
	class TestBase : public QObject
	{
		Q_OBJECT
	protected:
		QList<QImage> TestImages_;

		QMap<QSize, QList<QImage>> BenchImages_;

		static const int BenchRepsCount = 3;
	protected:
		uchar LMaxDiff (const QImage& image1, const QImage& image2)
		{
			if (image1.size () != image2.size ())
				return std::numeric_limits<uchar>::max ();

			uchar diff = 0;

			for (int y = 0; y < image1.height (); ++y)
			{
				const auto sl1 = image1.scanLine (y);
				const auto sl2 = image2.scanLine (y);

				for (int x = 0; x < image1.width () * 4; ++x)
					diff = std::max<uchar> (diff, std::abs (sl1 [x] - sl2 [x]));
			}

			return diff;
		}

		template<typename Ref, typename F, typename... Args>
		uchar CompareModifying (const QImage& image, Ref&& refFunc, F&& testFunc, Args... args)
		{
			auto ref = image;
			refFunc (ref, args...);
			auto avx = image;
			testFunc (avx, args...);

			return LMaxDiff (ref, avx);
		}

		template<typename F>
		void BenchmarkFunction (F&& f)
		{
			BenchmarkFunctionImpl (std::forward<F> (f), 0);
		}
	private:
		template<typename F, typename = std::result_of_t<F (const QImage&)>>
		void BenchmarkFunctionImpl (F&& f, int)
		{
			for (const auto& pair : Util::Stlize (BenchImages_))
			{
				const auto& list = pair.second;

				for (const auto& image : list)
					f (image);

				QElapsedTimer timer;
				timer.start ();

				int rep = 0;
				for (; rep < BenchRepsCount && timer.nsecsElapsed () < 50 * 1000 * 1000; ++rep)
					for (const auto& image : list)
						f (image);

				qDebug () << pair.first << ": " << timer.nsecsElapsed () / (1000 * rep * list.size ());
			}
		}

		template<typename F>
		void BenchmarkFunctionImpl (F&& f, float)
		{
			for (const auto& pair : Util::Stlize (BenchImages_))
			{
				auto list = pair.second;

				for (auto& image : list)
				{
					image.detach ();
					f (image);
				}

				uint64_t counter = 0;
				QElapsedTimer timer;
				timer.start ();

				for (int i = 0; i < BenchRepsCount; ++i)
					for (auto& image : list)
					{
						QElapsedTimer timer;
						timer.start ();

						f (image);

						counter += timer.nsecsElapsed ();
					}

				qDebug () << pair.first << ": " << counter / (1000 * BenchRepsCount * list.size ());
			}
		}
	private slots:
		void initTestCase ();
	};
}
}
}

#define CHECKFEATURE(x) \
	if (!Util::CpuFeatures {}.HasFeature (Util::CpuFeatures::Feature::x)) \
		QSKIP ("unsupported instruction set", SkipAll);
