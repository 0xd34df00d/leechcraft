/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "colorschemesmanager.h"
#include <optional>
#include <QDir>
#include <QSettings>
#include <QSet>
#include <QStandardPaths>
#include <QtDebug>
#include <qtermwidget.h>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>

namespace LC::Eleeminator
{
	namespace
	{
		using MaybeScheme_t = std::optional<ColorSchemesManager::Scheme>;

		MaybeScheme_t ParseScheme (const QString& filename)
		{
			QSettings settings { filename, QSettings::IniFormat };
			auto name = settings.value ("Description"_qs).toString ();
			if (name.isEmpty ())
				name = QFileInfo { filename }.baseName ();

			return { { name, filename } };
		}

		auto LoadKonsoleSchemes ()
		{
			QVector<ColorSchemesManager::Scheme> schemes;
			for (const auto& location : QStandardPaths::standardLocations (QStandardPaths::GenericDataLocation))
			{
				const auto& schemeFiles = QDir { location + "/konsole/" }.entryInfoList ({ "*.colorscheme"_qs });

				for (const auto& schemeFile : schemeFiles)
				{
					const auto& maybeScheme = ParseScheme (schemeFile.filePath ());
					if (maybeScheme)
						schemes << *maybeScheme;
				}
			}
			return schemes;
		}

		auto LoadTermWidgetSchemes ()
		{
			QVector<ColorSchemesManager::Scheme> schemes;
			for (const auto& name : QTermWidget::availableColorSchemes ())
				schemes << ColorSchemesManager::Scheme { name, name };
			return schemes;
		}

		void FilterDuplicates (QVector<ColorSchemesManager::Scheme>& schemes)
		{
			QSet<QString> names;

			for (auto i = schemes.begin (); i != schemes.end (); )
			{
				const auto normalized = QString { i->Name_ }.remove (' ').toLower ();

				if (names.contains (normalized))
					i = schemes.erase (i);
				else
				{
					names << normalized;
					++i;
				}
			}
		}
	}

	ColorSchemesManager::ColorSchemesManager (QObject* parent)
	: QObject { parent }
	{
		Schemes_ += LoadKonsoleSchemes ();
		Schemes_ += LoadTermWidgetSchemes ();
		FilterDuplicates (Schemes_);

		std::ranges::sort (Schemes_, {}, &Scheme::Name_);
	}

	QVector<ColorSchemesManager::Scheme> ColorSchemesManager::GetSchemes () const
	{
		return Schemes_;
	}
}
