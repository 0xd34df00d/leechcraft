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

#include "highlighter.h"
#include <QTextCodec>
#include "hunspell/hunspell.hxx"

namespace LeechCraft
{
namespace Azoth
{
namespace Rosenthal
{
	Highlighter::Highlighter (std::shared_ptr<Hunspell> hunspell, QTextDocument *parent)
	: QSyntaxHighlighter (parent)
	, Hunspell_ (hunspell)
	{
		SpellCheckFormat_.setUnderlineColor (QColor (Qt::red));
		SpellCheckFormat_.setUnderlineStyle (QTextCharFormat::SpellCheckUnderline);

		Codec_ = QTextCodec::codecForName (Hunspell_->get_dic_encoding ());
	}

	void Highlighter::UpdateHunspell (std::shared_ptr<Hunspell> hunspell)
	{
		Hunspell_ = hunspell;
	}

	void Highlighter::highlightBlock (const QString& text)
	{
		QRegExp sr ("\\W+");
		const QStringList& splitted = text.simplified ()
				.split (sr, QString::SkipEmptyParts);
		int prevStopPos = 0;
		Q_FOREACH (const QString& str, splitted)
		{
			if (str.size () <= 1 ||
					CheckWord (str))
				continue;

			const int pos = text.indexOf (str, prevStopPos);
			if (pos >= 0)
			{
				setFormat (pos, str.length (), SpellCheckFormat_);
				prevStopPos = pos + str.length ();
			}
		}
	}

	bool Highlighter::CheckWord (const QString& word)
	{
		const QByteArray& encoded = Codec_->fromUnicode (word);
		return Hunspell_->spell (encoded.data ());
	}
}
}
}
