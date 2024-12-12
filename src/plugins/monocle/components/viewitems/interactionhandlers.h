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
		virtual void DoubleClicked (QMouseEvent&) {}
	};

	class MovingInteraction final : public InteractionHandler
	{
	public:
		explicit MovingInteraction (PagesView&);
		explicit MovingInteraction (PagesView&, IDocument&, const QVector<PageGraphicsItem*>&);
	};

	class AreaSelectionInteraction final : public InteractionHandler
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Monocle::AreaSelectionInteraction)

		IDocument& Doc_;
		bool ShowOnNextRelease_ = false;
	public:
		explicit AreaSelectionInteraction (PagesView&, IDocument&, const QVector<PageGraphicsItem*>&);

		void Moved (QMouseEvent&) override;
		void Released (QMouseEvent&) override;
	};

	class TextSelectionInteraction final : public InteractionHandler
	{
	public:
		struct BoxRepr;

		using SelectionCornerInfo = std::pair<PageGraphicsItem*, PageRelativePos>;
	private:
		QMap<int, std::vector<BoxRepr>> Boxes_;
		std::optional<SelectionCornerInfo> SelectionStart_;
		IHaveTextContent& IHTC_;
		const QVector<PageGraphicsItem*> Pages_;
	public:
		explicit TextSelectionInteraction (PagesView&, IDocument&, const QVector<PageGraphicsItem*>&);
		~TextSelectionInteraction () override;

		void Pressed (QMouseEvent&) override;
		void Moved (QMouseEvent&) override;
		void Released (QMouseEvent&) override;
	private:
		void UpdateSelection (ViewAbsolutePos);

		void SelectOnPage (PageGraphicsItem&, PageRelativePos, PageRelativePos);

		void EnsureHasSelectionStart (ViewAbsolutePos pos);
		std::vector<BoxRepr>& LoadBoxes (PageGraphicsItem&);
		std::optional<SelectionCornerInfo> GetPageInfo (ViewAbsolutePos);

		void ClearBoxes ();
	};
}
