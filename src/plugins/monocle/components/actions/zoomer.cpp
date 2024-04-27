/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "zoomer.h"
#include <QAction>
#include <QComboBox>
#include <QtDebug>
#include <util/sll/qtutil.h>
#include <util/sll/visitor.h>

namespace LC::Monocle
{
	constexpr float Percent = 100;
	constexpr std::array Scales = { 0.1, 0.25, 0.33, 0.5, 0.66, 0.8, 1.0, 1.25, 1.5, 2., 3., 4., 5., 7.5, 10. };

	namespace
	{
		std::optional<ScaleMode> ScaleFromString (QString str)
		{
			str.remove ('%');
			str = str.trimmed ();

			bool ok = false;
			auto num = str.toDouble (&ok);
			if (!ok || num < 1)
			{
				qWarning () << "could not convert" << str << "to number";
				return {};
			}

			return FixedScale { num / Percent };
		}

		ScaleMode GetComboScaleMode (QComboBox& scales)
		{
			const auto idx = scales.currentIndex ();
			switch (idx)
			{
			case 0:
				return FitWidth {};
			case 1:
				return FitPage {};
			default:
				return FixedScale { scales.itemData (idx).toDouble () };
			}
		}
	}

	Zoomer::Zoomer (const ScaleGetter& scaleGetter, QObject *parent)
	: QObject { parent }
	, Scales_ { std::make_unique<QComboBox> () }
	, ZoomOut_ { std::make_unique<QAction> (tr ("Zoom out")) }
	, ZoomIn_ { std::make_unique<QAction> (tr ("Zoom in")) }
	, ScaleGetter_ { scaleGetter }
	{
		Scales_->setEditable (true);
		Scales_->setInsertPolicy (QComboBox::NoInsert);
		Scales_->addItem (tr ("Fit width"));
		Scales_->addItem (tr ("Fit page"));
		for (auto scale : Scales)
			Scales_->addItem (QString::number (scale * Percent) + '%', scale);
		Scales_->setCurrentIndex (0);
		connect (&*Scales_,
				&QComboBox::currentIndexChanged,
				this,
				[this] { NotifyScaleSelected (GetComboScaleMode (*Scales_)); });
		connect (&*Scales_,
				&QComboBox::editTextChanged,
				this,
				[this] (const QString& str)
				{
					if (Scales_->findText (str) == -1)
						if (const auto scale = ScaleFromString (str))
							NotifyScaleSelected (*scale);
				});

		ZoomOut_->setProperty ("ActionIcon", "zoom-out");
		ZoomOut_->setShortcut ("Ctrl+-"_qs);
		connect (&*ZoomOut_,
				&QAction::triggered,
				this,
				&Zoomer::ZoomOut);

		ZoomIn_->setProperty ("ActionIcon", "zoom-in");
		ZoomIn_->setShortcut ("Ctrl+="_qs);
		connect (&*ZoomIn_,
				&QAction::triggered,
				this,
				&Zoomer::ZoomIn);
	}

	Zoomer::~Zoomer () = default;

	QVector<ToolbarEntry> Zoomer::GetToolbarEntries () const
	{
		return { &*Scales_, &*ZoomOut_, &*ZoomIn_ };
	}

	void Zoomer::SetScaleMode (ScaleMode scaleMode)
	{
		const auto scaleBoxIndex = Util::Visit (scaleMode,
				[] (FitWidth) { return 0; },
				[] (FitPage) { return 1; },
				[&] (FixedScale fixed) { return Scales_->findData (fixed.Scale_); });
		if (scaleBoxIndex >= 0)
			Scales_->setCurrentIndex (scaleBoxIndex);
	}

	void Zoomer::ZoomOut ()
	{
		auto currentMatchingIndex = Scales_->currentIndex ();
		const int minIdx = 2;
		switch (Scales_->currentIndex ())
		{
		case 0:
		case 1:
		{
			const auto scale = ScaleGetter_ ();
			for (auto i = minIdx; i < Scales_->count (); ++i)
				if (Scales_->itemData (i).toDouble () > scale)
				{
					currentMatchingIndex = i;
					break;
				}

			if (currentMatchingIndex == Scales_->currentIndex ())
				currentMatchingIndex = Scales_->count () - 1;
			break;
		}
		}

		auto newIndex = std::max (currentMatchingIndex - 1, minIdx);
		Scales_->setCurrentIndex (newIndex);

		ZoomOut_->setEnabled (newIndex > minIdx);
		ZoomIn_->setEnabled (true);
	}

	void Zoomer::ZoomIn ()
	{
		const auto maxIdx = Scales_->count () - 1;

		auto newIndex = std::min (Scales_->currentIndex () + 1, maxIdx);
		switch (Scales_->currentIndex ())
		{
		case 0:
		case 1:
			const auto scale = ScaleGetter_ ();
			for (auto i = 2; i <= maxIdx; ++i)
				if (Scales_->itemData (i).toDouble () > scale)
				{
					newIndex = i;
					break;
				}
			if (Scales_->currentIndex () == newIndex)
				newIndex = maxIdx;
			break;
		}

		Scales_->setCurrentIndex (newIndex);

		ZoomOut_->setEnabled (true);
		ZoomIn_->setEnabled (newIndex < maxIdx);
	}

	void Zoomer::NotifyScaleSelected (ScaleMode scale)
	{
		emit scaleModeChanged (scale);
	}
}
