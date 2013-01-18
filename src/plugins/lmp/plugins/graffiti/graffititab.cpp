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

#include "graffititab.h"
#include <QFileSystemModel>

namespace LeechCraft
{
namespace LMP
{
namespace Graffiti
{
	GraffitiTab::GraffitiTab (ILMPProxy_ptr proxy, const TabClassInfo& tc, QObject *plugin)
	: LMPProxy_ (proxy)
	, TC_ (tc)
	, Plugin_ (plugin)
	, FSModel_ (new QFileSystemModel (this))
	{
		Ui_.setupUi (this);

		FSModel_->setRootPath (QDir::rootPath ());
		FSModel_->setFilter (QDir::Dirs);
		FSModel_->setReadOnly (true);
		Ui_.DirectoryTree_->setModel (FSModel_);
	}

	TabClassInfo GraffitiTab::GetTabClassInfo () const
	{
		return TC_;
	}

	QObject* GraffitiTab::ParentMultiTabs ()
	{
		return Plugin_;
	}

	void GraffitiTab::Remove ()
	{
		emit removeTab (this);
	}

	QToolBar* GraffitiTab::GetToolBar () const
	{
		return 0;
	}
}
}
}
