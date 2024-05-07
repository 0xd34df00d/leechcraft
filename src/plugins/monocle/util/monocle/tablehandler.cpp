/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "tablehandler.h"
#include <QTextTable>
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
			};
			return TableDims { rows, cols };
		}

		auto GetTableDimensions (const QDomElement& table)
		{
			auto tableDims = GetElemDimensions (table);
			if (const auto& thead = table.firstChildElement ("thead"_qs);
				!thead.isNull ())
				tableDims += GetElemDimensions (thead);
			if (const auto& tbody = table.firstChildElement ("tbody"_qs);
				!tbody.isNull ())
				tableDims += GetElemDimensions (tbody);
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

		QTextTable& CreateTable (const QDomElement& table, QTextCursor& cursor, const QTextFrameFormat& frameFmt)
		{
			const auto& dims = GetTableDimensions (table);
			return *cursor.insertTable (dims.Rows_, dims.Cols_, MakeTableFormat (frameFmt));
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
		if (elem.tagName () == "tr"_ql)
		{
			++Row_;
			Col_ = -1;
		}
		if (elem.tagName () == "td"_ql)
		{
			++Col_;
			Cursor_ = Table_.cellAt (Row_, Col_).firstCursorPosition ();
		}
	}
}
