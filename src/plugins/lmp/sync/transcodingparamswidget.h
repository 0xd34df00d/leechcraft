/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QWidget>
#include "ui_transcodingparamswidget.h"
#include "formats.h"

namespace LC
{
namespace LMP
{
	struct TranscodingParams;

	class TranscodingParamsWidget : public QWidget
	{
		Q_OBJECT

		Ui::TranscodingParamsWidget Ui_;
		std::shared_ptr<Formats> Formats_;
	public:
		TranscodingParamsWidget (QWidget* = 0);

		void SetMaskVisible (bool);

		TranscodingParams GetParams () const;
		void SetParams (const TranscodingParams&);
	private:
		Format_ptr GetCurrentFormat () const;
		Format::BitrateType GetCurrentBitrateType () const;
	private slots:
		void on_TranscodingFormat__currentIndexChanged ();
		void on_BitrateTypeBox__currentIndexChanged (int);
		void on_QualitySlider__valueChanged (int);
	};
}
}
