/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2011  Georg Rudoy
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

#ifndef PLUGINS_POSHUKU_PLUGINS_FATAPE_USERSCRIPT_H
#define PLUGINS_POSHUKU_PLUGINS_FATAPE_USERSCRIPT_H
#include <QObject>
#include <QMultiMap>
#include <QRegExp>
#include <QWebFrame>

namespace LeechCraft
{
namespace Poshuku
{
namespace FatApe
{
	class UserScript : public QObject
	{
		Q_OBJECT

		QString ScriptPath_;
		QRegExp MetadataRX_;
		QMultiMap<QString, QString> Metadata_;
		
		void ParseMetadata ();
		void BuildPatternsList (QList<QRegExp>& list, bool include = true) const;

	public:
		UserScript (const QString& scriptPath);
		UserScript (const UserScript& script);
		bool MatchToPage (const QString& pageUrl) const;
		void Inject (QWebFrame* frame) const;

			
    };
}
}
}
#endif
