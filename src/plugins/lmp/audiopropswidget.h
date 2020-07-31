/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QWidget>
#include "ui_audiopropswidget.h"

class QStandardItemModel;

namespace LC
{
namespace LMP
{
	struct MediaInfo;

	class AudioPropsWidget : public QWidget
	{
		Q_OBJECT

		Ui::AudioPropsWidget Ui_;
		QStandardItemModel *PropsModel_;
	public:
		AudioPropsWidget (QWidget* = 0);

		static AudioPropsWidget* MakeDialog ();

		void SetProps (const QString&);
		void SetProps (const MediaInfo&);
	private slots:
		void handleCopy ();
	};
}
}
