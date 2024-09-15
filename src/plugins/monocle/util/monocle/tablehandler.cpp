/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tablehandler.h"
#include <QTextTable>
#include <QtDebug>
#include "util/sll/domchildrenrange.h"
#include "util/sll/qtutil.h"

namespace LC::Monocle
{
	namespace
	{
		auto GetElemDimensions (const QDomElement& elem)
		{
			int rows = 0;
			int cols = 0;
			for (const auto& row : Util::DomChildren (elem, "tr"_qs))
			{
				++rows;
				const auto& colsElems = Util::DomChildren (row, "td"_qs);
				const auto thisCols = std::distance (std::begin (colsElems), std::end (colsElems));
				cols = std::max (cols, static_cast<int> (thisCols));
			}

			struct TableDims
			{
				int Rows_;
				int Cols_;

				TableDims& operator+= (TableDims other)
				{
					Rows_ += other.Rows_;
					Cols_ = std::max (Cols_, other.Cols_);
					return *this;
				}

				explicit operator bool () const
				{
					return Rows_ && Cols_;
				}
			};
			return TableDims { rows, cols };
		}

		auto GetTableDimensions (const QDomElement& table)
		{
			auto tableDims = GetElemDimensions (table);
			tableDims += GetElemDimensions (table.firstChildElement ("thead"_qs));
			tableDims += GetElemDimensions (table.firstChildElement ("tbody"_qs));
			if (!tableDims)
			{
				QByteArray str;
				QTextStream stream { &str };
				table.save (stream, 2);
				qWarning () << "table has no dimensions\n" << str;
			}
			return tableDims;
		}

		QTextTableFormat MakeTableFormat (const QTextFrameFormat& styledFrameFmt)
		{
			QTextTableFormat result;

			using enum QTextFormat::Property;
			constexpr std::array frameProps
			{
				CssFloat, PageBreakPolicy,
				FrameMargin, FramePadding, FrameWidth, FrameHeight,
				FrameTopMargin, FrameBottomMargin, FrameLeftMargin, FrameRightMargin,
			};
			for (auto prop : frameProps)
				result.setProperty (prop, styledFrameFmt.property (prop));

			result.setBorderCollapse (true);
			result.setWidth (QTextLength { QTextLength::PercentageLength, 100 });

			return result;
		}

		QDomElement IgnoreNonTableTags (QDomElement table)
		{
			if (table.isNull ())
				return table;

			auto firstChild = table.firstChildElement ();
			const auto& name = firstChild.tagName ();
			if (name == "thead"_qs || name == "tbody"_qs || name == "tr"_qs)
				return table;

			auto nextChild = firstChild.nextSiblingElement ();
			return nextChild.isNull () ? IgnoreNonTableTags (firstChild) : table;
		}

		QTextTable* CreateTable (QDomElement table, QTextCursor& cursor, const QTextFrameFormat& frameFmt)
		{
			table = IgnoreNonTableTags (table);
			const auto& dims = GetTableDimensions (table);
			if (!dims)
				return nullptr;
			return cursor.insertTable (dims.Rows_, dims.Cols_, MakeTableFormat (frameFmt));
		}
	}

	TableHandler::TableHandler (const QDomElement& table, QTextCursor& cursor, const QTextFrameFormat& frameFmt)
	: Cursor_ { cursor }
	, OrigFrame_ { *cursor.currentFrame () }
	, Table_ { CreateTable (table, cursor, frameFmt) }
	{
	}

	TableHandler::~TableHandler ()
	{
		Cursor_ = OrigFrame_.lastCursorPosition ();
	}

	void TableHandler::HandleElem (const QDomElement& elem)
	{
		if (!Table_)
			return;

		if (elem.tagName () == "tr"_ql)
		{
			++Row_;
			Col_ = -1;
		}
		if (elem.tagName () == "td"_ql)
		{
			++Col_;
			Cursor_ = Table_->cellAt (Row_, Col_).firstCursorPosition ();
		}
	}
}
