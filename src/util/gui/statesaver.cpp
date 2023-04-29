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
#include <xmlsettingsdialog/basesettingsmanager.h>

namespace LC::Util
{
	namespace
	{
		QList<int> RedistributeWidths (int totalWidth, QVector<std::optional<int>> initialWidths)
		{
			const int occupiedWidth = std::accumulate (initialWidths.begin (), initialWidths.end (), 0,
					[] (auto acc, auto elem) { return acc + elem.value_or (0); });
			const int missingCount = std::count (initialWidths.begin (), initialWidths.end (), std::nullopt);

			const auto widthPerMissing = missingCount ?
					std::max (1, (totalWidth - occupiedWidth) / missingCount) :
					0;

			QList<int> result;
			result.reserve (initialWidths.size ());
			for (const auto& w : initialWidths)
				result << w.value_or (widthPerMissing);
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
		auto widths = FromVariantList (params.XSM_.Property (params.Id_, {}).value<QVariantList> ());
		if (widths.isEmpty ())
			widths = RedistributeWidths (splitter.width (), params.InitialWidths_);

		splitter.setSizes (widths);

		QObject::connect (&splitter,
				&QSplitter::splitterMoved,
				[&xsm = params.XSM_, id = params.Id_, &splitter]
				{
					xsm.setProperty (id.c_str (), ToVariantList (splitter.sizes ()));
				});
	}
}
