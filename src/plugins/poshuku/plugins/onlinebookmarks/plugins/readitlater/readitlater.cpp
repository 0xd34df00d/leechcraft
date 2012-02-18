/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#include "readitlater.h"
#include <QIcon>
#include <util/util.h>
#include "readitlaterauthwidget.h"
#include "readitlaterservice.h"

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
namespace ReadItLater
{
	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Util::InstallTranslator ("poshuku_onlinebookmarks_readitlater");

		ReadItLaterService_.reset (new ReadItLaterService (proxy));

		connect (ReadItLaterService_.get (),
				SIGNAL (gotEntity (const LeechCraft::Entity&)),
				this,
				SIGNAL (gotEntity (const LeechCraft::Entity&)));
	}

	void Plugin::SecondInit ()
	{
		ReadItLaterService_->Prepare ();
	}

	void Plugin::Release ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Poshuku.OnlineBookmarks.ReadItLater";
	}

	QString Plugin::GetName () const
	{
		return "Poshuku OB: Read It Later";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("Sync local bookmarks with your Read It Later account.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> classes;
		classes << "org.LeechCraft.Plugins.Poshuku.Plugins.OnlineBookmarks.IServicePlugin";
		return classes;
	}

	QObject* Plugin::GetObject ()
	{
		return this;
	}

	QObject* Plugin::GetBookmarksService () const
	{
		return ReadItLaterService_.get ();
	}

}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_poshuku_onlinebookmarks_readitlater,
		LeechCraft::Poshuku::OnlineBookmarks::ReadItLater::Plugin);
