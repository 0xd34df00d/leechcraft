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
		static QStringList replacements;
		int modsSize = mods.size ();
		if (replacements.isEmpty ())
			for (int i = 0; i < modsSize; ++i)
				replacements << QString (".%1.").arg (-i - 1);

		for (int i = 0; i < modsSize; ++i)
			version.replace (mods.at (i), replacements.at (i));

		return version;
	}

	bool IsVersionLess (const QString& leftVer, const QString& rightVer)
	{
		if (leftVer == rightVer)
			return false;

		QString leftNum = Numerize (leftVer);
		QString rightNum = Numerize (rightVer);

#ifdef VERSIONCOMPARATOR_DEBUG
		qDebug () << leftVer << "->" << leftNum
				<< rightVer << "->" << rightNum;
#endif

		auto leftParts = leftNum.splitRef ('.', Qt::SkipEmptyParts);
		auto rightParts = rightNum.splitRef ('.', Qt::SkipEmptyParts);

		int minSize = std::min (leftParts.size (), rightParts.size ());

#ifdef VERSIONCOMPARATOR_DEBUG
		qDebug () << leftParts << rightParts;
#endif

		for (int i = 0; i < minSize; ++i)
		{
			int left = leftParts.at (i).toInt ();
			int right = rightParts.at (i).toInt ();
#ifdef VERSIONCOMPARATOR_DEBUG
			qDebug () << left << right;
#endif
			if (left < right)
				return true;
			else if (left > right)
				return false;
		}

		if (leftParts.size () >= rightParts.size ())
			return false;

		for (int i = minSize; i < rightParts.size (); ++i)
			if (rightParts [i] != "0")
				return true;

		return false;
	}
}
}
