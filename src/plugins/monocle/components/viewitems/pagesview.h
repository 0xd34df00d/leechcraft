/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QGraphicsView>
#include "components/layout/positions.h"
#include "components/viewitems/interactionhandlers.h"

namespace LC::Monocle
{
	class IDocument;
	class InteractionHandler;

	class PagesView : public QGraphicsView
	{
		Q_OBJECT

		std::unique_ptr<InteractionHandler> InteractionHandler_ = std::make_unique<MovingInteraction> (*this);

		IDocument *Doc_ = nullptr;
	public:
		using QGraphicsView::QGraphicsView;

		void SetDocument (IDocument*);

		template<typename T>
		void SetInteractionHandler ()
		{
			InteractionHandler_ = std::make_unique<T> (*this);
		}

		void CenterOn (SceneAbsolutePos);

		SceneAbsolutePos GetCurrentCenter () const;
		SceneAbsolutePos GetViewportTrimmedCenter (const QGraphicsItem&) const;
	protected:
		void mouseMoveEvent (QMouseEvent*) override;
		void mouseReleaseEvent (QMouseEvent*) override;
		void resizeEvent (QResizeEvent*) override;
	signals:
		void sizeChanged ();
	};
}
