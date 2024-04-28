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
#include <util/sll/unreachable.h>
#include <util/sll/visitor.h>

namespace LC::Monocle
{
	constexpr float Percent = 100;
	constexpr int NonFixedModesCount = 3;
	constexpr std::array ZoomLevels = { 0.1, 0.25, 0.33, 0.5, 0.66, 0.8, 1.0, 1.25, 1.5, 2., 3., 4., 5., 7.5, 10. };

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
			case 2:
				qWarning () << "unexpected separator idx";
				return FitPage {};
			default:
				return FixedScale { scales.itemData (idx).toDouble () };
			}
		}

		enum class ZoomDelta : std::int8_t
		{
			Out = -1,
			In = 1,
		};

		std::optional<int> GetNearZoomIndexUnclamped (double curZoom, ZoomDelta delta)
		{
			const auto pos = std::lower_bound (ZoomLevels.begin (), ZoomLevels.end (), curZoom);
			if (pos == ZoomLevels.end () || *pos == curZoom)
				return (pos - ZoomLevels.begin ()) + static_cast<int> (delta);

			switch (delta)
			{
			case ZoomDelta::Out:
				return (pos - ZoomLevels.begin ()) - 1;
			case ZoomDelta::In:
				return (pos - ZoomLevels.begin ());
			}
			Util::Unreachable ();
		}

		std::optional<int> ClampToZoomLevelsCount (int idx)
		{
			if (idx < 0 || idx >= static_cast<int> (ZoomLevels.size ()))
				return {};
			return idx;
		}

		std::optional<int> GetNearScaleBoxIndex (QComboBox& scalesBox, double curZoom, ZoomDelta delta)
		{
			const auto curIdx = scalesBox.currentIndex ();
			if (curIdx >= NonFixedModesCount)
				return ClampToZoomLevelsCount (curIdx - NonFixedModesCount + static_cast<int> (delta))
						.transform ([] (int val) { return val + NonFixedModesCount; });

			return GetNearZoomIndexUnclamped (curZoom, delta)
					.and_then (&ClampToZoomLevelsCount);
		}
	}

	Zoomer::Zoomer (ScaleGetter scaleGetter, QObject *parent)
	: QObject { parent }
	, Scales_ { std::make_unique<QComboBox> () }
	, ZoomOut_ { std::make_unique<QAction> (tr ("Zoom out")) }
	, ZoomIn_ { std::make_unique<QAction> (tr ("Zoom in")) }
	, ScaleGetter_ { std::move (scaleGetter) }
	{
		Scales_->setEditable (true);
		Scales_->setInsertPolicy (QComboBox::NoInsert);
		Scales_->addItem (tr ("Fit width"));
		Scales_->addItem (tr ("Fit page"));
		Scales_->insertSeparator (Scales_->count ());
		Q_ASSERT (Scales_->count () == NonFixedModesCount);
		for (auto scale : ZoomLevels)
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
				[this]
				{
					if (const auto idx = GetNearScaleBoxIndex (*Scales_, ScaleGetter_ (), ZoomDelta::Out))
						Scales_->setCurrentIndex (*idx);
				});

		ZoomIn_->setProperty ("ActionIcon", "zoom-in");
		ZoomIn_->setShortcut ("Ctrl+="_qs);
		connect (&*ZoomIn_,
				&QAction::triggered,
				this,
				[this]
				{
					if (const auto idx = GetNearScaleBoxIndex (*Scales_, ScaleGetter_ (), ZoomDelta::In))
						Scales_->setCurrentIndex (*idx);
				});
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

	void Zoomer::NotifyScaleSelected (ScaleMode scale)
	{
		emit scaleModeChanged (scale);

		const auto curScale = ScaleGetter_ ();
		ZoomOut_->setEnabled (GetNearScaleBoxIndex (*Scales_, curScale, ZoomDelta::Out).has_value ());
		ZoomIn_->setEnabled (GetNearScaleBoxIndex (*Scales_, curScale, ZoomDelta::In).has_value ());
	}
}
