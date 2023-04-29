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
	}

	void SetupStateSaver (QSplitter& splitter, const StateSaverParams& params)
	{
		auto restorer = [&splitter, params, firstTime = true] (QEvent*, QObject& pThis) mutable
		{
			if (!firstTime)
				return false;
			firstTime = false;

			auto widths = FromVariantList (params.XSM_.Property (params.Id_, {}).value<QVariantList> ());
			if (widths.isEmpty ())
				widths = Visit (params.Initial_,
						[&] (const auto& value) { return RedistributeWidths (splitter.width (), value); });

			splitter.setSizes (widths);
			pThis.deleteLater ();
			return false;
		};
		splitter.installEventFilter (MakeLambdaEventFilter<QEvent::Resize> (std::move (restorer), splitter));

		QObject::connect (&splitter,
				&QSplitter::splitterMoved,
				[&xsm = params.XSM_, id = params.Id_, &splitter]
				{
					xsm.setProperty (id.c_str (), ToVariantList (splitter.sizes ()));
				});
	}
}
