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
		struct ViewConfig;

		explicit InteractionHandler (PagesView&, const ViewConfig&);
	public:
		virtual void Pressed (QMouseEvent&, IDocument&) {}
		virtual void Moved (QMouseEvent&, IDocument&) {}
		virtual void Released (QMouseEvent&, IDocument&) {}
	};

	class MovingInteraction final : public InteractionHandler
	{
	public:
		explicit MovingInteraction (PagesView&);
	};

	class AreaSelectionInteraction final : public InteractionHandler
	{
		Q_DECLARE_TR_FUNCTIONS (LC::Monocle::AreaSelectionInteraction)

		PagesView& View_;
		bool ShowOnNextRelease_ = false;
	public:
		explicit AreaSelectionInteraction (PagesView&);

		void Moved (QMouseEvent&, IDocument&) override;
		void Released (QMouseEvent&, IDocument&) override;
	};
}
