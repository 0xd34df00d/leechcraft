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

namespace LC::Util
{
	template<typename T>
	class FlatItemsModel;
}

namespace LC::LMP
{
	struct MediaInfo;

	class AudioPropsWidget : public QWidget
	{
		Q_DECLARE_TR_FUNCTIONS (LC::LMP::AudioPropsWidget)

		Ui::AudioPropsWidget Ui_ {};
		Util::FlatItemsModel<QPair<QString, QString>> * const PropsModel_;
	public:
		explicit AudioPropsWidget (QWidget* = nullptr);

		static AudioPropsWidget* MakeDialog ();

		void SetProps (const QString&);
		void SetProps (const MediaInfo&);
	private:
		void CopyRow () const;
	};
}
