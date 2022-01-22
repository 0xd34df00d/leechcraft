/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "imagecollectiondialog.h"
#include <QtDebug>
#include "imageinfosmodel.h"

namespace LC::LHTR
{
	ImageCollectionDialog::ImageCollectionDialog (const RemoteImageInfos_t& infos, QWidget *parent)
	: QDialog { parent }
	, Infos_ { infos }
	{
		Ui_.setupUi (this);

		auto model = new ImageInfosModel (Infos_, this);
		Ui_.Images_->setModel (model);

		if (infos.isEmpty ())
			return;

		const auto& sample = infos.first ();

		const auto addSize = [this] (const QSize& size, const QString& str, PreviewSize enumValue)
		{
			if (!size.isValid ())
				return;

			Ui_.PreviewSize_->addItem (str
					.arg (size.width ())
					.arg (size.height ()));
			Sizes_ << enumValue;
		};

		addSize (sample.ThumbSize_, tr ("Thumbnail (%1×%2)"), PreviewSize::Thumb);
		addSize (sample.PreviewSize_, tr ("Preview (%1×%2)"), PreviewSize::Preview);
		addSize (sample.FullSize_, tr ("Full (%1×%2)"), PreviewSize::Full);
	}

	RemoteImageInfos_t ImageCollectionDialog::GetInfos () const
	{
		return Infos_;
	}

	ImageCollectionDialog::Position ImageCollectionDialog::GetPosition () const
	{
		switch (Ui_.Position_->currentIndex ())
		{
		case 0:
			return Position::Center;
		case 1:
			return Position::Left;
		case 2:
			return Position::Right;
		case 3:
			return Position::LeftWrap;
		case 4:
			return Position::RightWrap;
		}

		qWarning () << "unknown position"
				<< Ui_.Position_->currentIndex ();
		return Position::Center;
	}

	bool ImageCollectionDialog::PreviewsAreLinks () const
	{
		return Ui_.PreviewsAreLinks_->checkState () == Qt::Checked;
	}

	ImageCollectionDialog::PreviewSize ImageCollectionDialog::GetPreviewSize () const
	{
		return Sizes_.value (Ui_.PreviewSize_->currentIndex (), PreviewSize::None);
	}
}
