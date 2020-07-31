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
#include <qtermwidget.h>
#include <util/sll/prelude.h>

namespace LC
{
namespace Eleeminator
{
	ColorSchemesManager::ColorSchemesManager (QObject* parent)
	: QObject { parent }
	{
		LoadKonsoleSchemes ();

		Schemes_ += Util::Map (QTermWidget::availableColorSchemes (),
				[] (const QString& name) { return Scheme { name, name }; });

		FilterDuplicates ();

		std::sort (Schemes_.begin (), Schemes_.end (), Util::ComparingBy (&Scheme::Name_));
	}

	QList<ColorSchemesManager::Scheme> ColorSchemesManager::GetSchemes () const
	{
		return Schemes_;
	}

	namespace
	{
		QStringList CollectSchemes (const QString& dir)
		{
			return Util::Map (QDir { dir }.entryList ({ "*.colorscheme" }),
					[&dir] (QString str) { return str.prepend (dir); });
		}

		using MaybeScheme_t = std::optional<ColorSchemesManager::Scheme>;

		MaybeScheme_t ParseScheme (const QString& filename)
		{
			QSettings settings { filename, QSettings::IniFormat };
			settings.setIniCodec ("UTF-8");
			auto name = settings.value ("Description").toString ();
			if (name.isEmpty ())
				name = QFileInfo { filename }.baseName ();

			return { { name, filename } };
		}
	}

	void ColorSchemesManager::LoadKonsoleSchemes ()
	{
		const auto& pathCandidates = Util::Map (QStandardPaths::standardLocations (QStandardPaths::GenericDataLocation),
				[] (const QString& str) { return str + "/konsole/"; });

		const auto& filenames = Util::ConcatMap (pathCandidates, &CollectSchemes);
		Schemes_ += Util::Map (Util::Filter (Util::Map (filenames, &ParseScheme),
						[] (const MaybeScheme_t& scheme) { return static_cast<bool> (scheme); }),
					[] (const MaybeScheme_t& scheme) { return *scheme; });
	}

	void ColorSchemesManager::FilterDuplicates ()
	{
		QSet<QString> names;

		for (auto i = Schemes_.begin (); i != Schemes_.end (); )
		{
			const auto normalized = QString { i->Name_ }.remove (' ').toLower ();

			if (names.contains (normalized))
				i = Schemes_.erase (i);
			else
			{
				names << normalized;
				++i;
			}
		}
	}
}
}
