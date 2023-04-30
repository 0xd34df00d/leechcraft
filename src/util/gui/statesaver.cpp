/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "statesaver.h"
#include <numeric>
#include <QSplitter>
#include <util/sll/lambdaeventfilter.h>
#include <util/sll/throttle.h>
#include <util/sll/visitor.h>
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC::Util
{
	namespace
	{
		QList<int> RedistributeWidths (int totalWidth, const Widths& widths)
		{
			const int occupiedWidth = std::accumulate (widths.begin (), widths.end (), 0,
					[] (auto acc, auto elem) { return acc + elem.value_or (0); });
			const int missingCount = std::count (widths.begin (), widths.end (), std::nullopt);

			const auto widthPerMissing = missingCount ?
					std::max (1, (totalWidth - occupiedWidth) / missingCount) :
					0;

			QList<int> result;
			result.reserve (widths.size ());
			for (const auto& w : widths)
				result << w.value_or (widthPerMissing);
			return result;
		}

		QList<int> RedistributeWidths (int totalWidth, const Factors& factors)
		{
			const auto totalFactors = std::accumulate (factors.begin (), factors.end (), 0);

			QList<int> result;
			result.reserve (factors.size ());
			for (auto size : factors)
				result << size * totalWidth / totalFactors;
			return result;
		}

		QList<int> RedistributeWidths (int totalWidth, const InitialDistr& initial)
		{
			return Visit (initial, [&] (const auto& value) { return RedistributeWidths (totalWidth, value); });
		}

		QList<int> FromVariantList (const QVariantList& list)
		{
			QList<int> result;
			result.reserve (list.size ());
			for (const auto& var : list)
			{
				bool ok = true;
				result << var.toInt (&ok);
				if (!ok)
					return {};
			}
			return result;
		}

		QVariantList ToVariantList (const QList<int>& list)
		{
			QVariantList result;
			result.reserve (list.size ());
			for (auto num : list)
				result << num;
			return result;
		}

		template<typename F>
		auto SingleRun (F&& f)
		{
			return [f = std::forward<F> (f)] (QEvent*, QObject& pThis) mutable
			{
				std::invoke (std::forward<F> (f));
				pThis.deleteLater ();
				return false;
			};
		}

		template<typename F>
		void OnResize (QWidget& widget, F&& f)
		{
			widget.installEventFilter (MakeLambdaEventFilter<QEvent::Resize> (SingleRun (std::move (f)), widget));
		}
	}

	void SetupStateSaver (QSplitter& splitter, const StateSaverParams& params)
	{
		OnResize (splitter,
				[&splitter, params]
				{
					auto widths = FromVariantList (params.XSM_.Property (params.Id_, {}).value<QVariantList> ());
					if (widths.isEmpty ())
						widths = RedistributeWidths (splitter.width (), params.Initial_);
					splitter.setSizes (widths);
				});

		using namespace std::chrono_literals;
		QObject::connect (&splitter,
				&QSplitter::splitterMoved,
				Throttled (1s, &splitter, [&xsm = params.XSM_, id = params.Id_, &splitter]
					{
						xsm.setProperty (id.c_str (), ToVariantList (splitter.sizes ()));
					}));
	}
}
