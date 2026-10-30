/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>
#include <QMap>
#include <QPointer>
#include "interfaces/monocle/ihavetextcontent.h"
#include "components/layout/positions.h"

class QMouseEvent;

namespace LC::Monocle
{
	class IDocument;
	class PagesView;
	class TextSelectionItem;

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
		struct Position
		{
			qsizetype Box_;
			qsizetype Letter_;
		};

		struct Selection
		{
			Position Start_;
			Position End_;
		};

		struct PageText
		{
			QVector<TextBox> Boxes_;
			std::optional<Selection> Selection_;
			QPointer<TextSelectionItem> Overlay_;
		};
	private:
		IHaveTextContent& IHTC_;
		const QVector<QPointer<PageGraphicsItem>> Pages_;
		QMap<int, PageText> Texts_;
		std::optional<PageWithRelativePos> SelectionStart_;
		bool IsSelectionGesture_ = false;
	public:
		explicit TextSelectionInteraction (PagesView&, IDocument&, const QVector<PageGraphicsItem*>&);
		~TextSelectionInteraction () override;

		void Pressed (QMouseEvent&) override;
		void Moved (QMouseEvent&) override;
		void Released (QMouseEvent&) override;

		void DoubleClicked (QMouseEvent&) override;
	private:
		void UpdateSelection (ViewAbsolutePos);
		void UpdateOverlays ();

		void SelectOnPage (int page, PageRelativePos, PageRelativePos);
		QString GetSelectedText (bool preserveNewlines) const;

		void EnsureHasSelectionStart (ViewAbsolutePos pos);
		PageText& LoadText (int page);
		std::optional<PageWithRelativePos> GetPageInfo (ViewAbsolutePos) const;

		void ClearSelection ();
	};
}
