/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "highlighter.h"
#include <QtDebug>

namespace LC
{
namespace AnHero
{
namespace CrashProcess
{
	void Highlighter::highlightBlock (const QString& text)
	{
		if (text.startsWith ("Thread "))
			setFormat (0, text.size (), QColor ("#B3925D"));
		else if (text.startsWith ("#"))
			ParseBTLine (text);
	}

	namespace
	{
		int FindOneOf (const QString& text, int from, const QList<QByteArray>& variants)
		{
			for (const auto& marker : variants)
			{
				const auto tmpIdx = text.indexOf (marker, from);
				if (tmpIdx == -1)
					continue;

				return tmpIdx + marker.size ();
			}

			return -1;
		}
	}

	void Highlighter::ParseBTLine (const QString& text)
	{

		auto numberEnd = text.indexOf (' ');
		setFormat (0, numberEnd, QColor ("#555555"));

		while (text.at (numberEnd) == ' ')
			++numberEnd;

		const QString sighandlerMarker ("signal handler called");
		const auto sigIdx = text.indexOf (sighandlerMarker);
		if (sigIdx > 0)
		{
			QTextCharFormat fmt;
			fmt.setFontWeight (QFont::Bold);
			fmt.setForeground ({ "#E20800" });
			setFormat (sigIdx, sighandlerMarker.size (), fmt);
			return;
		}

		int funcStartIdx = -1;

		const bool isAddr = text [numberEnd] == '0' && text [numberEnd + 1] == 'x';
		if (isAddr)
		{
			const auto addrEnd = text.indexOf (' ', numberEnd);
			setFormat (numberEnd, addrEnd - numberEnd, QColor ("#BBBBBB"));

			funcStartIdx = text.indexOf (' ', addrEnd + 1) + 1;
		}
		else
			funcStartIdx = FindOneOf (text, numberEnd, { " to ", " in " });

		if (funcStartIdx != -1)
			ParseFunction (text, funcStartIdx);
		else
			ParseFunction (text, numberEnd);
	}

	void Highlighter::ParseFunction (const QString& text, int funcStartIdx)
	{
		const auto doubleColon = text.lastIndexOf ("::", text.indexOf ('(', funcStartIdx));
		if (doubleColon == -1)
			ParseCFunction (text, funcStartIdx);
		else
			ParseCppFunction (text, funcStartIdx, doubleColon);
	}

	void Highlighter::ParseCFunction (const QString& text, int funcStartIdx)
	{
		const auto funcEndIdx = text.indexOf (' ', funcStartIdx);
		if (text [funcStartIdx] == '?' && text [funcStartIdx + 1] == '?')
			setFormat (funcStartIdx, funcEndIdx - funcStartIdx, QColor ("#E85752"));
		else
			setFormat (funcStartIdx, funcEndIdx - funcStartIdx, QColor ("#462886"));

		ParseRest (text, funcEndIdx);
	}

	void Highlighter::ParseCppFunction (const QString& text, int funcStartIdx, int doubleColonIdx)
	{
		const auto parenIdx = text.indexOf ('(', doubleColonIdx);
		setFormat (funcStartIdx, doubleColonIdx - funcStartIdx, QColor ("#008C00"));

		QTextCharFormat fmt;
		fmt.setFontWeight (QFont::Bold);
		fmt.setForeground ({ "#644A9B" });
		setFormat (doubleColonIdx + 2, parenIdx - doubleColonIdx - 2, fmt);

		ParseRest (text, parenIdx);
	}

	void Highlighter::ParseRest (const QString& text, int fromAround)
	{
		const auto restIdx = FindOneOf (text, fromAround, { " at ", " from " });

		auto colonIdx = text.lastIndexOf (':');
		if (colonIdx < restIdx)
			colonIdx = -1;

		if (colonIdx == -1)
			colonIdx = text.size ();

		setFormat (restIdx, colonIdx - restIdx, QColor ("#0057AE"));
	}
}
}
}
