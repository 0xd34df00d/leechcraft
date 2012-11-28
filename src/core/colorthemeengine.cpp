/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "colorthemeengine.h"
#include <algorithm>
#include <QFile>
#include <QStringList>
#include <QApplication>
#include <QDir>
#include <QtDebug>
#include <QSettings>

namespace LeechCraft
{
	ColorThemeEngine::ColorThemeEngine ()
	{
		SetTheme ("crafty");
	}

	ColorThemeEngine& ColorThemeEngine::Instance ()
	{
		static ColorThemeEngine engine;
		return engine;
	}

	namespace
	{
		QStringList GetCandidates ()
		{
			QStringList candidates;
#ifdef Q_OS_WIN32
			candidates << QApplication::applicationDirPath () + "/share/leechcraft/themes/";
#elif defined (Q_OS_MAC)
			candidates << QApplication::applicationDirPath () + "/../Resources/share/leechcraft/themes";
#else
			candidates << "/usr/local/share/leechcraft/themes/"
					<< "/usr/share/leechcraft/themes/";
#endif
			return candidates;
		}

		QStringList FindThemes ()
		{
			QStringList result;
			for (const auto& candidate : GetCandidates ())
			{
				QDir dir (candidate);
				const auto& list = dir.entryList (QDir::Dirs | QDir::NoDotAndDotDot);
				result += list;
			}
			result.removeDuplicates ();
			std::sort (result.begin (), result.end ());
			return result;
		}
	}

	QStringList ColorThemeEngine::ListThemes () const
	{
		return FindThemes ();
	}

	namespace
	{
		QColor ParseColor (const QVariant& var)
		{
			const auto& elems = var.toStringList ();
			if (elems.size () != 3)
			{
				qWarning () << Q_FUNC_INFO
						<< "wrong color"
						<< var
						<< elems;
				return QColor ();
			}

			return QColor (elems.at (0).toInt (),
					elems.at (1).toInt (),
					elems.at (2).toInt ());
		}

		void UpdateColor (QPalette& palette, QSettings& settings,
				QPalette::ColorGroup group, QPalette::ColorRole role,
				const QString& settingsGroup, const QString& key)
		{
			settings.beginGroup ("Colors:" + settingsGroup);
			palette.setColor (group, role, ParseColor (settings.value (key)));
			settings.endGroup ();
		}

		QPalette UpdatePalette (QPalette palette, QSettings& settings)
		{
			auto updateColor = [&palette, &settings] (QPalette::ColorRole role,
					QPalette::ColorGroup group, const QString& sg, const QString& key)
			{
				UpdateColor (palette, settings, group, role, sg, key);
			};
			auto updateAll = [updateColor] (QPalette::ColorRole role,
					const QString& sg, const QString& key)
			{
				updateColor (role, QPalette::Active, sg, key);
				updateColor (role, QPalette::Inactive, sg, key);
				updateColor (role, QPalette::Disabled, sg, key);
			};

			updateAll (QPalette::Window, "Window", "BackgroundNormal");
			updateAll (QPalette::WindowText, "Window", "ForegroundNormal");
			updateAll (QPalette::BrightText, "Window", "ForegroundPositive");
			updateAll (QPalette::Link, "Window", "ForegroundLink");
			updateAll (QPalette::LinkVisited, "Window", "ForegroundVisited");

			updateAll (QPalette::Base, "View", "BackgroundNormal");
			updateAll (QPalette::AlternateBase, "View", "BackgroundAlternate");
			updateAll (QPalette::Text, "View", "ForegroundNormal");

			updateAll (QPalette::Button, "Button", "BackgroundNormal");
			updateAll (QPalette::ButtonText, "Button", "ForegroundNormal");

			updateAll (QPalette::ToolTipBase, "Tooltip", "BackgroundNormal");
			updateAll (QPalette::ToolTipText, "Tooltip", "ForegroundNormal");

			return palette;
		}
	}

	void ColorThemeEngine::SetTheme (const QString& themeName)
	{
		const auto& candidates = GetCandidates ();

		QString themePath;
		for (const auto& path : candidates)
			if (QFile::exists (path + themeName))
			{
				themePath = path + themeName;
				break;
			}

		if (themePath.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no theme"
					<< themeName;
			return;
		}

		QSettings settings (themePath + "/colors.rc", QSettings::IniFormat);
		if (settings.childGroups ().isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "error opening colors file for"
					<< themeName;
			return;
		}

		qDebug () << settings.childGroups ();

		auto palette = UpdatePalette (StartupPalette_, settings);
		QApplication::setPalette (palette);

		QSettings qmlSettings (themePath + "/qml.rc", QSettings::IniFormat);
		FillQML (qmlSettings);
	}

	void ColorThemeEngine::FillQML (QSettings& settings)
	{
		QMLColors_.clear ();

		for (const auto& group : settings.childGroups ())
		{
			settings.beginGroup (group);
			auto& hash = QMLColors_ [group];
			for (const auto& key : settings.childKeys ())
				hash [key] = ParseColor (settings.value (key));
			settings.endGroup ();
		}
	}
}
