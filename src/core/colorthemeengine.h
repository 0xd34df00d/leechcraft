/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include <QPalette>
#include <QHash>
#include <interfaces/core/icolorthememanager.h>

class QAbstractItemModel;
class QSettings;

namespace LC
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
		QHash<QByteArray, QHash<QByteArray, QColor>> QMLColors_;

		Util::ResourceLoader *Loader_;

		ColorThemeEngine ();
	public:
		static ColorThemeEngine& Instance ();

		QColor GetQMLColor (const QByteArray& section, const QByteArray& key);
		QObject* GetQObject ();

		QAbstractItemModel* GetThemesModel () const;
		void SetTheme (const QString&);
	private:
		void FillQML (QSettings&);
	signals:
		void themeChanged ();
	};
}
