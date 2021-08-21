/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "versioncomparator.h"
#include <QStringList>
#include <QtDebug>

namespace LC
{
namespace LackMan
{
	void ParseDotted (QVector<int>& result, const QStringRef& dotted)
	{
		for (const auto& part : dotted.split ('.', Qt::SkipEmptyParts))
		{
			bool isInt = false;
			if (const auto num = part.toInt (&isInt);
					isInt)
				result.append (num);
			else
				qWarning () << Q_FUNC_INFO << "unable to parse" << part;
		}
	}

	void ParseModifier (QVector<int>& result, const QStringRef& modifier)
	{
		static const QStringList mods { "rc", "pre", "beta", "alpha" };

		for (int i = 0; i < mods.size (); ++i)
		{
			if (!modifier.startsWith (mods.at (i)))
				continue;

			result.append (-i - 1);
			const auto& leftover = modifier.mid (mods.at (i).size ());
			if (!leftover.isEmpty ())
			{
				bool ok = false;
				if (const auto num = leftover.toInt (&ok);
					ok)
					result.append (num);
				else
					qWarning () << Q_FUNC_INFO
							<< "unable to parse"
							<< leftover
							<< "of"
							<< modifier;
			}
			return;
		}

		qWarning () << Q_FUNC_INFO
				<< "unknown modifier"
				<< modifier;
	}

	QVector<int> Numerize (const QString& version)
	{
		const auto& comps = version.splitRef ('-', Qt::SkipEmptyParts);
		if (comps.size () > 2 || comps.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO << "unexpected components:" << comps;
			return {};
		}

		QVector<int> result;
		result.reserve (version.count ('.') + 1 + comps.size ());
		ParseDotted (result, comps.at (0));
		if (comps.size () == 2)
			ParseModifier (result, comps.at (1));
		return result;
	}

	bool IsVersionLess (const QString& leftVer, const QString& rightVer)
	{
		if (leftVer == rightVer)
			return false;

		const auto& leftParts = Numerize (leftVer);
		const auto& rightParts = Numerize (rightVer);

		for (int i = 0; i < leftParts.size () || i < rightParts.size (); ++i)
		{
			int left = leftParts.value (i, 0);
			int right = rightParts.value (i, 0);
			if (left == right)
				continue;

			return left < right;
		}

		return false;
	}
}
}
