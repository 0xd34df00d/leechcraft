/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "feedsexportdialog.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/iiconthememanager.h>
#include <util/gui/util.h>
#include <util/models/checkableproxymodel.h>
#include <util/sll/qtutil.h>
#include "exportutils.h"
#include "feed.h"

namespace LC::Aggregator
{
	class ChannelsExportModel : public Util::CheckableProxyModel<IDType_t>
	{
	public:
		using CheckableProxyModel::CheckableProxyModel;

		QVariant data (const QModelIndex& index, int role) const override
		{
			switch (role)
			{
			case Qt::BackgroundRole:
			case Qt::ForegroundRole:
				return {};
			}

			return CheckableProxyModel::data (index, role);
		}
	};

	FeedsExportDialog::FeedsExportDialog (QAbstractItemModel& model, QWidget *parent)
	: QDialog { parent }
	, ChannelsModel_ { std::make_unique<ChannelsExportModel> (ChannelID) }
	{
		Ui_.setupUi (this);
		setWindowIcon (GetProxyHolder ()->GetIconThemeManager ()->GetPluginIcon ());

		ManageLastPath ({ *Ui_.File_, "FeedsExportLastPath", QDir::homePath () + "/feeds.opml", *this });

		ChannelsModel_->setSourceModel (&model);

		Ui_.Channels_->setModel (ChannelsModel_.get ());
		Ui_.Channels_->header ()->resizeSections (QHeaderView::ResizeToContents);
		Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (false);

		connect (Ui_.Browse_,
				&QPushButton::released,
				this,
				&FeedsExportDialog::Browse);
		QTimer::singleShot (0, this,
				[this]
				{
					if (!Browse ())
						reject ();
				});

		connect (Ui_.File_,
				&QLineEdit::textEdited,
				this,
				[this] (const QString& text)
				{
					Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (!text.isEmpty ());
				});

		connect (Ui_.SelectAll_,
				&QPushButton::released,
				ChannelsModel_.get (),
				&Util::CheckableProxyModel<IDType_t>::CheckAll);
		connect (Ui_.SelectNone_,
				&QPushButton::released,
				ChannelsModel_.get (),
				&Util::CheckableProxyModel<IDType_t>::CheckNone);
	}

	FeedsExportDialog::~FeedsExportDialog () = default;

	QString FeedsExportDialog::GetDestination () const
	{
		return Ui_.File_->text ();
	}

	QString FeedsExportDialog::GetTitle () const
	{
		return Ui_.Title_->text ();
	}

	QString FeedsExportDialog::GetOwner () const
	{
		return Ui_.Owner_->text ();
	}

	QString FeedsExportDialog::GetOwnerEmail () const
	{
		return Ui_.OwnerEmail_->text ();
	}

	QSet<IDType_t> FeedsExportDialog::GetSelectedFeeds () const
	{
		return ChannelsModel_->GetChecked ();
	}

	bool FeedsExportDialog::Browse ()
	{
		const auto& filename = QFileDialog::getSaveFileName (this,
				tr ("Export as"),
				Ui_.File_->text (),
				Util::MakeFileDialogFilter ({ { tr ("OPML files"), "opml"_ql }, { tr ("All files"), "*"_ql } }));
		if (filename.isEmpty ())
			return false;

		Ui_.File_->setText (filename);
		Ui_.ButtonBox_->button (QDialogButtonBox::Save)->setEnabled (true);
		return true;
	}
}
