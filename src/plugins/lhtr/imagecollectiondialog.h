/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QDialog>
#include <interfaces/data/iimgsource.h>
#include <interfaces/core/icoreproxy.h>
#include "ui_imagecollectiondialog.h"

namespace LC
{
namespace LHTR
{
	class ImageCollectionDialog : public QDialog
	{
		Q_OBJECT

		Ui::ImageCollectionDialog Ui_;
	public:
		enum class PreviewSize
		{
			None,
			Thumb,
			Preview,
			Full
		};
	private:
		RemoteImageInfos_t Infos_;
		QList<PreviewSize> Sizes_;
	public:
		enum class Position
		{
			Center,
			Left,
			Right,
			LeftWrap,
			RightWrap
		};

		explicit ImageCollectionDialog (const RemoteImageInfos_t&, QWidget* = nullptr);

		RemoteImageInfos_t GetInfos () const;
		Position GetPosition () const;
		bool PreviewsAreLinks () const;
		PreviewSize GetPreviewSize () const;
	};
}
}
