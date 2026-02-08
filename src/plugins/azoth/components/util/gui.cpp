/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "gui.h"
#include <QApplication>
#include <QMainWindow>
#include <QPalette>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/irootwindowsmanager.h>
#include "../../xmlsettingsmanager.h"

namespace LC::Azoth
{
	QList<QColor> GenerateColors (const QString& coloring, QColor bg)
	{
		QList<QColor> result;
		if (XmlSettingsManager::Instance ().property ("OverrideHashColors").toBool ())
		{
			result = XmlSettingsManager::Instance ().property ("OverrideColorsList").value<decltype (result)> ();
			if (!result.isEmpty ())
				return result;
		}

		auto compatibleColors = [] (const QColor& c1, const QColor& c2) -> bool
		{
			int dR = c1.red () - c2.red ();
			int dG = c1.green () - c2.green ();
			int dB = c1.blue () - c2.blue ();

			double dV = std::abs (c1.value () - c2.value ());
			double dC = std::sqrt (0.2126 * dR * dR + 0.7152 * dG * dG + 0.0722 * dB * dB);

			if ((dC < 80. && dV > 100.) ||
					(dC < 110. && dV <= 100. && dV > 10.) ||
					(dC < 125. && dV <= 10.))
				return false;

			return true;
		};

		if (coloring == "hash" || coloring.isEmpty ())
		{
			if (!bg.isValid ())
				bg = QApplication::palette ().color (QPalette::Base);

			int alpha = bg.alpha ();

			QColor color;
			for (int hue = 0; hue < 360; hue += 18)
			{
				color.setHsv (hue, 255, 255, alpha);
				if (compatibleColors (color, bg))
					result << color;
				color.setHsv (hue, 255, 170, alpha);
				if (compatibleColors (color, bg))
					result << color;
			}
		}
		else
			for (const auto& str : QStringView { coloring }.split (' ', Qt::SkipEmptyParts))
				result << QColor { str };

		return result;
	}

	QString GetNickColor (const QString& nick, const QList<QColor>& colors)
	{
		if (colors.isEmpty ())
			return "green";

		int hash = 0;
		for (int i = 0; i < nick.length (); ++i)
		{
			const QChar& c = nick.at (i);
			hash += c.toLatin1 () ?
					c.toLatin1 () :
					c.unicode ();
			hash += nick.length ();
		}
		hash = std::abs (hash);
		const auto& nc = colors.at (hash % colors.size ());
		return nc.name ();
	}

	QWidget* GetDialogParent ()
	{
		return GetProxyHolder ()->GetRootWindowsManager ()->GetPreferredWindow ();
	}

	QString GetActivityIconName (const QString& general, const QString& specific)
	{
		return (general + ' ' + specific).trimmed ().replace (' ', '_');
	}

	QString StateToString (State st)
	{
		switch (st)
		{
		case SOnline:
			return QObject::tr ("Online");
		case SChat:
			return QObject::tr ("Free to chat");
		case SAway:
			return QObject::tr ("Away");
		case SDND:
			return QObject::tr ("Do not disturb");
		case SXA:
			return QObject::tr ("Not available");
		case SOffline:
			return QObject::tr ("Offline");
		default:
			return QObject::tr ("Error");
		}
	}
}
