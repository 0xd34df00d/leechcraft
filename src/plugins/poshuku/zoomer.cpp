/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "zoomer.h"
#include <QWheelEvent>
#include <QAction>

namespace LC
{
namespace Poshuku
{
	Zoomer::Zoomer (const ZoomGetter_f& getter, const ZoomSetter_f& setter,
			QObject *parent, const QList<qreal>& zooms)
	: QObject { parent }
	, Zooms_ { zooms }
	, Getter_ { getter }
	, Setter_ { setter }
	{
	}

	void Zoomer::SetActionsTriple (QAction *in, QAction *out, QAction *reset)
	{
		connect (in,
				SIGNAL (triggered ()),
				this,
				SLOT (zoomIn ()));
		connect (out,
				SIGNAL (triggered ()),
				this,
				SLOT (zoomOut ()));
		connect (reset,
				SIGNAL (triggered ()),
				this,
				SLOT (zoomReset ()));
	}

	void Zoomer::InstallScrollFilter (QObject *obj, const std::function<bool (QWheelEvent*)>& cond)
	{
		class ScrollEF : public QObject
		{
			const std::function<bool (QWheelEvent*)> Cond_;
			Zoomer * const Zoomer_;
		public:
			ScrollEF (const std::function<bool (QWheelEvent*)>& cond,
					Zoomer *zoomer, QObject *parent = nullptr)
			: QObject { parent }
			, Cond_ { cond }
			, Zoomer_ { zoomer }
			{
			}

			bool eventFilter (QObject*, QEvent *event) override
			{
				if (event->type () != QEvent::Wheel)
					return false;

				const auto we = static_cast<QWheelEvent*> (event);
				if (!Cond_ (we))
					return false;

				qreal degrees = we->angleDelta ().y () / 8.;
				qreal delta = degrees / 150.;
				Zoomer_->Setter_ (Zoomer_->Getter_ () + delta);

				return true;
			}
		};

		obj->installEventFilter (new ScrollEF { cond, this, obj });
	}

	int Zoomer::LevelForZoom (qreal zoom) const
	{
		int i = Zooms_.indexOf (zoom);

		if (i >= 0)
			return i;

		for (i = 0; i < Zooms_.size (); ++i)
			if (zoom <= Zooms_ [i])
				break;

		if (i == Zooms_.size ())
			return i - 1;

		if (i == 0)
			return i;

		if (zoom - Zooms_ [i - 1] > Zooms_ [i] - zoom)
			return i;
		else
			return i - 1;
	}

	void Zoomer::SetZoom (qreal zoom)
	{
		Setter_ (zoom);
		emit zoomChanged ();
	}

	void Zoomer::zoomIn ()
	{
		int i = LevelForZoom (Getter_ ());

		if (i < Zooms_.size () - 1)
			SetZoom (Zooms_ [i + 1]);
	}

	void Zoomer::zoomOut ()
	{
		int i = LevelForZoom (Getter_ ());

		if (i > 0)
			SetZoom (Zooms_ [i - 1]);
	}

	void Zoomer::zoomReset ()
	{
		SetZoom (1);
	}
}
}
