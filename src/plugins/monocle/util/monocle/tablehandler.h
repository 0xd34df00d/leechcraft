/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QDomElement;

class QTextCursor;
class QTextFrame;
class QTextFrameFormat;
class QTextTable;

namespace LC::Monocle
{
	class TableHandler
	{
		QTextCursor& Cursor_;
		QTextFrame& OrigFrame_;
		QTextTable& Table_;

		int Row_ = -1;
		int Col_ = -1;
	public:
		TableHandler (const QDomElement& table, QTextCursor& cursor, const QTextFrameFormat& frameFmt);

		TableHandler (const TableHandler&) = delete;
		TableHandler (TableHandler&&) = delete;
		TableHandler& operator= (const TableHandler&) = delete;
		TableHandler& operator= (TableHandler&&) = delete;

		~TableHandler ();

		void HandleElem (const QDomElement& elem);
	};
}
