/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include "interfaces/monocle/ihavetextcontent.h"
#include "components/layout/positions.h"

class QGraphicsRectItem;
class QMouseEvent;

namespace LC::Monocle
{
	class IDocument;
	class PagesView;

	class InteractionHandler : public QObject
	{
	protected:
		PagesView& View_;

		struct ViewConfig;

		explicit InteractionHandler (PagesView&, const ViewConfig&);
	public:
		virtual void Pressed (QMouseEvent&) {}
		virtual void Moved (QMouseEvent&) {}
		virtual void Released (QMouseEvent&) {}
	};

	class MovingInteraction final : public InteractionHandler
	{
	public:
		explicit MovingInteraction (PagesView&);
		explicit MovingInteraction (PagesView&, IDocument&);
	};

	class AreaSelectionInteraction final : public InteractionHandler
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Monocle::AreaSelectionInteraction)

		IDocument& Doc_;
		bool ShowOnNextRelease_ = false;
	public:
		explicit AreaSelectionInteraction (PagesView&, IDocument&);

		void Moved (QMouseEvent&) override;
		void Released (QMouseEvent&) override;
	};

	class TextSelectionInteraction final : public InteractionHandler
	{
	public:
		struct BoxInfo;
	private:
		QMap<int, std::vector<BoxInfo>> Boxes_;

		using SelectionCornerInfo = std::pair<PageGraphicsItem*, PageRelativePos>;
		std::optional<SelectionCornerInfo> SelectionStart_;

		IHaveTextContent& IHTC_;
	public:
		explicit TextSelectionInteraction (PagesView&, IDocument&);
		~TextSelectionInteraction () override;

		void Pressed (QMouseEvent&) override;
		void Moved (QMouseEvent&) override;
		void Released (QMouseEvent&) override;
	private:
		void EnsureHasSelectionStart (QPointF pos);

		std::vector<BoxInfo>& LoadBoxes (PageGraphicsItem&);

		std::optional<SelectionCornerInfo> GetPageInfo (QPointF);
	};
}
