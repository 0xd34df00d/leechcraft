/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "cloudwidget.h"
#include <algorithm>
#include <iterator>
#include <interfaces/lmp/icloudstorageplugin.h>
#include "sync/devicesuploadmodel.h"
#include "sync/clouduploadmanager.h"
#include "sync/transcodingparams.h"
#include "core.h"
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	CloudWidget::CloudWidget (QWidget *parent)
	: QWidget (parent)
	, DevUploadModel_ (new DevicesUploadModel (this))
	{
		Ui_.setupUi (this);
		Ui_.TranscodingOpts_->SetMaskVisible (false);

		DevUploadModel_->setSourceModel (Core::Instance ().GetLocalCollection ()->GetCollectionModel ());
		Ui_.OurCollection_->setModel (DevUploadModel_);

		Ui_.SyncTabs_->setEnabled (false);

		connect (&Core::Instance (),
				SIGNAL (cloudStoragePluginsChanged ()),
				this,
				SLOT (handleCloudStoragePlugins ()));
		handleCloudStoragePlugins ();

		Ui_.TSProgress_->hide ();
		Ui_.UploadProgress_->hide ();

		connect (Core::Instance ().GetCloudUploadManager (),
				SIGNAL (uploadLog (QString)),
				this,
				SLOT (appendUpLog (QString)));

		connect (Core::Instance ().GetCloudUploadManager (),
				SIGNAL (transcodingProgress (int, int)),
				this,
				SLOT (handleTranscodingProgress (int, int)));
		connect (Core::Instance ().GetCloudUploadManager (),
				SIGNAL (uploadProgress (int, int)),
				this,
				SLOT (handleUploadProgress (int, int)));
	}

	void CloudWidget::on_CloudSelector__activated (int idx)
	{
		Ui_.AccountSelector_->clear ();
		Ui_.SyncTabs_->setEnabled (false);
		if (idx < 0)
			return;

		auto cloud = qobject_cast<ICloudStoragePlugin*> (Clouds_.at (idx));
		const auto& accounts = cloud->GetAccounts ();
		if (accounts.isEmpty ())
			return;

		Ui_.AccountSelector_->addItems (accounts);
		Ui_.SyncTabs_->setEnabled (true);
	}

	void CloudWidget::handleCloudStoragePlugins ()
	{
		Ui_.CloudSelector_->clear ();

		Clouds_ = Core::Instance ().GetCloudStoragePlugins ();
		Q_FOREACH (QObject *cloudObj, Clouds_)
		{
			auto cloud = qobject_cast<ICloudStoragePlugin*> (cloudObj);
			Ui_.CloudSelector_->addItem (cloud->GetCloudIcon (), cloud->GetCloudName ());

			connect (cloudObj,
					SIGNAL (accountsChanged ()),
					this,
					SLOT (handleAccountsChanged ()),
					Qt::UniqueConnection);
		}

		if (!Clouds_.isEmpty ())
			on_CloudSelector__activated (0);
	}

	void CloudWidget::handleAccountsChanged ()
	{
		const int idx = Ui_.CloudSelector_->currentIndex ();
		if (idx < 0 || sender () != Clouds_.at (idx))
			return;

		on_CloudSelector__activated (idx);
	}

	void CloudWidget::on_UploadButton__released ()
	{
		const int idx = Ui_.CloudSelector_->currentIndex ();
		const auto& accName = Ui_.AccountSelector_->currentText ();
		if (idx < 0 || accName.isEmpty ())
			return;

		const auto& selected = DevUploadModel_->GetSelectedIndexes ();
		QStringList paths;
		std::transform (selected.begin (), selected.end (), std::back_inserter (paths),
				[] (const QModelIndex& idx) { return idx.data (LocalCollection::Role::TrackPath).toString (); });
		paths.removeAll (QString ());

		Ui_.UploadLog_->clear ();

		auto cloud = qobject_cast<ICloudStoragePlugin*> (Clouds_.at (idx));
		Core::Instance ().GetCloudUploadManager ()->AddFiles (cloud,
				accName, paths, Ui_.TranscodingOpts_->GetParams ());
	}

	void CloudWidget::appendUpLog (QString text)
	{
		text.prepend (QTime::currentTime ().toString ("[HH:mm:ss.zzz] "));
		Ui_.UploadLog_->append ("<code>" + text + "</code>");
	}

	void CloudWidget::handleTranscodingProgress (int done, int total)
	{
		Ui_.TSProgress_->setVisible (done < total);
		Ui_.TSProgress_->setMaximum (total);
		Ui_.TSProgress_->setValue (done);
	}

	void CloudWidget::handleUploadProgress (int done, int total)
	{
		Ui_.UploadProgress_->setVisible (done < total);
		Ui_.UploadProgress_->setMaximum (total);
		Ui_.UploadProgress_->setValue (done);
	}
}
}
