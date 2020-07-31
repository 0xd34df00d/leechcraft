/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

class QPainter;

namespace LC
{
namespace Monocle
{
	/** @brief Interface for documents supporting optimized painting.
	 *
	 * This interface should be implemented by IDocument objects that can
	 * paint better than putting the image returned by the
	 * IDocument::RenderPage() method to a QPainter.
	 *
	 * @sa IDocument
	 */
	class ISupportPainting
	{
	public:
		virtual ~ISupportPainting () {}

		/** @brief Paints a given page to the given painter.
		 *
		 * @param[in] painter The painter to paint on.
		 * @param[in] page The page index to paint on.
		 * @param[in] xScale The X-axis scale which should be used during
		 * painting.
		 * @param[in] yScale The Y-axis scale which should be used during
		 * painting.
		 */
		virtual void PaintPage (QPainter *painter, int page, double xScale, double yScale) = 0;
	};
}
}

Q_DECLARE_INTERFACE (LC::Monocle::ISupportPainting,
		"org.LeechCraft.Monocle.ISupportPainting/1.0")
