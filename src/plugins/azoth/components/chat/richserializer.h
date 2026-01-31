/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>

class QTextBlockFormat;
class QTextCharFormat;
class QTextDocument;

namespace LC::Azoth
{
	class RichSerializer final
	{
		const QStringList StockFamilies_;

		QString Xhtml_;
		bool HasCustomFormatting_ = false;
	public:
		explicit RichSerializer (const QTextDocument& doc);

		bool HasCustomFormatting () const;
		QString GetXhtml () const;
	private:
		QPair<QString, QString> GetBlockTags (const QTextBlockFormat&);

		QStringList GetSpanStyle (const QTextCharFormat&) const;
		QPair<QStringList, QStringList> GetFragmentTags (const QTextCharFormat&);
	};
}
