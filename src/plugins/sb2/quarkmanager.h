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

#pragma once

#include <QObject>
#include <QUrl>
#include <xmlsettingsdialog/xmlsettingsdialog.h>

namespace LeechCraft
{
struct QuarkComponent;

namespace SB2
{
	class ViewManager;
	class QuarkSettingsManager;

	class QuarkManager : public QObject
	{
		ViewManager * const ViewMgr_;

		const QUrl URL_;

		Util::XmlSettingsDialog_ptr XSD_;
		QuarkSettingsManager *SettingsManager_;

		QString Name_;
		QString ID_;
		QStringList Areas_;
	public:
		QuarkManager (const QuarkComponent&, ViewManager*);

		QString GetID () const;
		QString GetName () const;

		bool IsValidArea () const;

		bool HasSettings () const;
		void ShowSettings ();
	private:
		QString GetSuffixedName (const QString&) const;
		void ParseManifest ();
		void CreateSettings ();
	};

	typedef std::shared_ptr<QuarkManager> QuarkManager_ptr;
}
}
