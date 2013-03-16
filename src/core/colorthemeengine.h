/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
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

#pragma once

#include <QObject>
#include <QPalette>
#include <QHash>
#include <interfaces/core/icolorthememanager.h>

class QAbstractItemModel;
class QSettings;

namespace LeechCraft
{
	namespace Util
	{
		class ResourceLoader;
	}

	class ColorThemeEngine : public QObject
						   , public IColorThemeManager
	{
		Q_OBJECT
		Q_INTERFACES (IColorThemeManager)

		QPalette StartupPalette_;
		QHash<QString, QHash<QString, QColor>> QMLColors_;

		Util::ResourceLoader *Loader_;

		ColorThemeEngine ();
	public:
		static ColorThemeEngine& Instance ();

		QColor GetQMLColor (const QString& section, const QString& key);
		QObject* GetQObject ();

		QAbstractItemModel* GetThemesModel () const;
		void SetTheme (const QString&);
	private:
		void FillQML (QSettings&);
	signals:
		void themeChanged ();
	};
}
