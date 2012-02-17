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

#ifndef PLUGINS_AZOTH_PLUGINS_HIGHLIGHTER_H
#define PLUGINS_AZOTH_PLUGINS_HIGHLIGHTER_H
#include <memory>
#include <QSyntaxHighlighter>
#include <QTextFormat>

class Hunspell;
class QTextCodec;

namespace LeechCraft
{
namespace Azoth
{
namespace Rosenthal
{
	class Highlighter : public QSyntaxHighlighter
	{
		Q_OBJECT

		std::shared_ptr<Hunspell> Hunspell_;
		QTextCharFormat SpellCheckFormat_;
		QTextCodec *Codec_;
	public:
		Highlighter (std::shared_ptr<Hunspell>, QTextDocument*);

		void UpdateHunspell (std::shared_ptr<Hunspell>);
	protected:
		void highlightBlock (const QString&);
	private:
		bool CheckWord (const QString&);
	};
}
}
}

#endif
