/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "colorthemeengine.h"
#include <algorithm>
#include <map>
#include <optional>
#include <QFile>
#include <QStringList>
#include <QApplication>
#include <QMainWindow>
#include <QDir>
#include <QtDebug>
#include <QSettings>
#include <util/sys/resourceloader.h>
#include "core.h"
#include "rootwindowsmanager.h"

namespace LC
{
	ColorThemeEngine::ColorThemeEngine ()
	: Loader_ (new Util::ResourceLoader ("themes/", this))
	{
		Loader_->AddLocalPrefix ();
		Loader_->AddGlobalPrefix ();
		Loader_->SetCacheParams (1, 0);
	}

	ColorThemeEngine& ColorThemeEngine::Instance ()
	{
		static ColorThemeEngine engine;
		return engine;
	}

	QColor ColorThemeEngine::GetQMLColor (const QString& section, const QString& key)
	{
		return QMLColors_ [section] [key];
	}

	QObject* ColorThemeEngine::GetQObject ()
	{
		return this;
	}

	namespace
	{
		QPalette::ColorRole ColorRoleFromStr (const QString& str)
		{
			static const std::map<QString, QPalette::ColorRole> map =
			{
				{ "Window", QPalette::Window },
				{ "WindowText", QPalette::WindowText },
				{ "BrightText", QPalette::BrightText },
				{ "Base", QPalette::Base },
				{ "AlternateBase", QPalette::AlternateBase },
				{ "Text", QPalette::Text },
				{ "ToolTipBase", QPalette::ToolTipBase },
				{ "ToolTipText", QPalette::ToolTipText },
				{ "Button", QPalette::Button },
				{ "ButtonText", QPalette::ButtonText },
				{ "Light", QPalette::Light },
				{ "Midlight", QPalette::Midlight },
				{ "Dark", QPalette::Dark },
				{ "Mid", QPalette::Mid },
				{ "Shadow", QPalette::Shadow },
				{ "Highlight", QPalette::Highlight },
				{ "HighlightedText", QPalette::HighlightedText }
			};

			const auto pres = map.find (str);
			if (pres == map.end ())
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown color"
						<< str;
				return QPalette::Window;
			}
			return pres->second;
		}

		QString ParseArg (const QString& str)
		{
			return str.section ('(', 1, 1).section (')', 0, 0).trimmed ();
		}

		template<typename F>
		void WithValue (QColor& color, F&& f)
		{
			qreal h, s, v, a;
			color.getHsvF (&h, &s, &v, &a);
			v = f (v);
			color.setHsvF (h, s, v, a);
		}

		QColor Modify (QColor color, const QString& str)
		{
			if (str.isEmpty ())
				return color;

			if (str.startsWith ("darker"))
			{
				const auto coeff = ParseArg (str).toInt () / 100.;
				WithValue (color, [coeff] (qreal v) { return v * (1 - coeff); });
				return color;
			}
			else if (str.startsWith ("lighter"))
			{
				const auto coeff = ParseArg (str).toInt () / 100.;
				WithValue (color, [coeff] (qreal v) { return v + coeff * (1 - v); });
				return color;
			}
			else
			{
				qWarning () << Q_FUNC_INFO
						<< "unknown function"
						<< str;
				return color;
			}
		}

		std::optional<QColor> ParsePaletteColor (const QString& str)
		{
			static const QString paletteMarker { "Palette." };
			if (!str.startsWith (paletteMarker))
				return {};

			const auto& role = ColorRoleFromStr (str.section ('.', 1, 1));
			const auto& color = QApplication::palette ().color (role);
			return Modify (color, str.section ('.', 2, 2));
		}

		QColor ParseColor (const QVariant& var)
		{
			const auto& str = var.toString ();
			if (const auto& color = ParsePaletteColor (str))
				return *color;

			const auto& elems = var.toStringList ();
			if (elems.size () < 3 || elems.size () > 4)
			{
				qWarning () << Q_FUNC_INFO
						<< "wrong color"
						<< var
						<< elems;
				return QColor ();
			}

			return QColor (elems.at (0).toInt (),
					elems.at (1).toInt (),
					elems.at (2).toInt (),
					elems.value (3, "255").toInt ());
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

			updateAll (QPalette::Highlight, "Selection", "BackgroundNormal");
			updateAll (QPalette::HighlightedText, "Selection", "ForegroundNormal");

			return palette;
		}
	}

	QAbstractItemModel* ColorThemeEngine::GetThemesModel () const
	{
		return Loader_->GetSubElemModel ();
	}

	void ColorThemeEngine::SetTheme (const QString& themeName)
	{
		const auto& themePath = Loader_->GetPath (QStringList (themeName));

		if (themePath.isEmpty ())
		{
			qWarning () << Q_FUNC_INFO
					<< "no theme"
					<< themeName;
			return;
		}

		auto palette = StartupPalette_;
		if (QFile::exists (themePath + "/colors.rc"))
		{
			QSettings settings (themePath + "/colors.rc", QSettings::IniFormat);
			if (settings.childGroups ().isEmpty ())
			{
				qWarning () << Q_FUNC_INFO
						<< "error opening colors file for"
						<< themeName;
				return;
			}

			palette = UpdatePalette (StartupPalette_, settings);
		}
		QApplication::setPalette (palette);
		const auto rootWinMgr = Core::Instance ().GetRootWindowsManager ();
		for (int i = 0; i < rootWinMgr->GetWindowsCount (); ++i)
		{
			const auto win = rootWinMgr->GetMainWindow (i);
			win->setPalette (palette);

			for (auto w : win->findChildren<QWidget*> ())
				w->setPalette (palette);
		}

		QSettings qmlSettings (themePath + "/qml.rc", QSettings::IniFormat);
		FillQML (qmlSettings);

		emit themeChanged ();
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

		auto fixup = [this, &settings] (const QString& section,
				const QString& name, const QString& fallback) -> void
		{
			auto& sec = QMLColors_ [section];
			if (sec.contains (name))
				return;

			qWarning () << Q_FUNC_INFO
					<< settings.fileName ()
					<< "lacks"
					<< (section + "_" + name)
					<< "; falling back to"
					<< fallback;
			sec [name] = sec [fallback];
		};

		fixup ("ToolButton", "HoveredTopColor", "SelectedTopColor");
		fixup ("ToolButton", "HoveredBottomColor", "SelectedBottomColor");
	}
}
