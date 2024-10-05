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

		using InteractionHandler_ptr = std::unique_ptr<InteractionHandler>;
		std::function<InteractionHandler_ptr ()> InteractionHandlerFactory_ { [this] { return std::make_unique<MovingInteraction> (*this); } };
		InteractionHandler_ptr InteractionHandler_ = InteractionHandlerFactory_ ();

		IDocument *Doc_ = nullptr;
	public:
		using QGraphicsView::QGraphicsView;

		void SetDocument (IDocument*);

		template<typename T>
		void SetInteractionHandler ()
		{
			InteractionHandlerFactory_ = [this] () -> InteractionHandler_ptr
			{
				try
				{
					if (Doc_)
						return std::make_unique<T> (*this, *Doc_);
				}
				catch (const std::exception& e)
				{
					qCritical () << Q_FUNC_INFO << "unable to create the interaction handler:" << e.what ();
				}

				return std::make_unique<MovingInteraction> (*this);
			};
			InteractionHandler_ = InteractionHandlerFactory_ ();
		}

		void CenterOn (SceneAbsolutePos);

		SceneAbsolutePos GetCurrentCenter () const;
		SceneAbsolutePos GetViewportTrimmedCenter (const QGraphicsItem&) const;
	protected:
		void mousePressEvent(QMouseEvent*) override;
		void mouseMoveEvent (QMouseEvent*) override;
		void mouseReleaseEvent (QMouseEvent*) override;
		void resizeEvent (QResizeEvent*) override;
	signals:
		void sizeChanged ();
	};
}
