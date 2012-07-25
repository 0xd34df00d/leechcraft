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
#include <interfaces/lmp/icloudstorageplugin.h>
#include "devsync/devicesuploadmodel.h"
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
		}

		if (!Clouds_.isEmpty ())
			on_CloudSelector__activated (0);
	}
}
}
