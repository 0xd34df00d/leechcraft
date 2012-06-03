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

#include "releaseswidget.h"
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/media/irecentreleases.h>
#include "core.h"

namespace LeechCraft
{
namespace LMP
{
	ReleasesWidget::ReleasesWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);

		Providers_ = Core::Instance ().GetProxy ()->GetPluginsManager ()->
				GetAllCastableTo<Media::IRecentReleases*> ();
		Q_FOREACH (auto prov, Providers_)
			Ui_.InfoProvider_->addItem (prov->GetServiceName ());

		Ui_.InfoProvider_->setCurrentIndex (-1);

		connect (Ui_.InfoProvider_,
				SIGNAL (activated (int)),
				this,
				SLOT (request ()));
		connect (Ui_.WithRecs_,
				SIGNAL (toggled (bool)),
				this,
				SLOT (request ()));
	}

	void ReleasesWidget::request ()
	{
		const auto idx = Ui_.InfoProvider_->currentIndex ();
		if (idx < 0)
			return;

		const bool withRecs = Ui_.WithRecs_->checkState () == Qt::Checked;
		Providers_.at (idx)->RequestRecentReleases (15, withRecs);
	}
}
}
