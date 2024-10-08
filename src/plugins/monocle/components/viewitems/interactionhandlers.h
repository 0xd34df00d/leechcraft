/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QCoreApplication>

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
}
