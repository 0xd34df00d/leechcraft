/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <memory>
#include <variant>
#include <QObject>
#include "common.h"

class QAction;
class QComboBox;
class QWidget;

namespace LC::Monocle
{
	class Zoomer : public QObject
	{
		Q_OBJECT

		const std::unique_ptr<QComboBox> Scales_;
		const std::unique_ptr<QAction> ZoomOut_;
		const std::unique_ptr<QAction> ZoomIn_;
	public:
		using ScaleGetter = std::function<double ()>;
	private:
		const ScaleGetter ScaleGetter_;
	public:
		explicit Zoomer (const ScaleGetter&, QObject* = nullptr);
		~Zoomer () override;

		QVector<ToolbarEntry> GetToolbarEntries () const;

		void SetScaleMode (ScaleMode);
	private:
		void HandleScaleChosen (int);
		void ZoomOut ();
		void ZoomIn ();
	signals:
		void scaleModeChanged (ScaleMode);
	};
}
