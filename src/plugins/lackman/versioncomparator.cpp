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
	QString Numerize (QString version)
	{
		static const QStringList mods { "-rc", "-pre", "-beta", "-alpha" };
		const int modsCount = mods.size ();
		static const QStringList replacements = [modsCount]
				{
					QStringList result;
					for (int i = 0; i < modsCount; ++i)
						result << QStringLiteral (".%1.").arg (-i - 1);
					return result;
				} ();

		for (int i = 0; i < modsCount; ++i)
			version.replace (mods.at (i), replacements.at (i));

		return version;
	}

	bool IsVersionLess (const QString& leftVer, const QString& rightVer)
	{
		if (leftVer == rightVer)
			return false;

		const auto& leftNum = Numerize (leftVer);
		const auto& rightNum = Numerize (rightVer);

#ifdef VERSIONCOMPARATOR_DEBUG
		qDebug () << leftVer << "->" << leftNum
				<< rightVer << "->" << rightNum;
#endif

		const auto& padStr = QStringLiteral (".0");

		auto leftParts = leftNum.splitRef ('.', Qt::SkipEmptyParts);
		auto rightParts = rightNum.splitRef ('.', Qt::SkipEmptyParts);
		auto pad = [&padStr] (QVector<QStringRef>& vec, const QVector<QStringRef>& target)
		{
			const auto sizeDiff = target.size () - vec.size ();
			if (sizeDiff > 0)
				vec.append ({ sizeDiff, QStringRef { &padStr } });
		};
		pad (leftParts, rightParts);
		pad (rightParts, leftParts);

#ifdef VERSIONCOMPARATOR_DEBUG
		qDebug () << leftParts << rightParts;
#endif

		for (int i = 0; i < leftParts.size (); ++i)
		{
			int left = leftParts.at (i).toInt ();
			int right = rightParts.at (i).toInt ();
#ifdef VERSIONCOMPARATOR_DEBUG
			qDebug () << left << right;
#endif
			if (left < right)
				return true;
			if (left > right)
				return false;
		}

		return false;
	}
}
}
