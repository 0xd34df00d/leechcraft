/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
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

#include "vgrabber.h"
#include <QIcon>
#include <QMessageBox>
#include <plugininterface/util.h>
#include "findproxy.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			void Plugin::Init (ICoreProxy_ptr)
			{
				Translator_.reset (Util::InstallTranslator ("vgrabber"));
			}

			void Plugin::Release ()
			{
				Translator_.reset ();
			}

			QString Plugin::GetName () const
			{
				return "vGrabber";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("vkontakte.ru music/video grabber.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon (":/resources/images/vgrabber.svg");
			}

			QStringList Plugin::Provides () const
			{
				return QStringList ();
			}

			QStringList Plugin::Needs () const
			{
				return QStringList ("http");
			}

			QStringList Plugin::Uses () const
			{
				return QStringList ();
			}

			void Plugin::SetProvider (QObject*, const QString&)
			{
			}

			QStringList Plugin::GetCategories () const
			{
				return QStringList ("vkontakte.ru music");
			}

			IFindProxy_ptr Plugin::GetProxy (const Request& req)
			{
				boost::shared_ptr<FindProxy> fp (new FindProxy (req));
				connect (fp.get (),
						SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::DownloadEntity&,
								int*, QObject**)));
				connect (fp.get (),
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
				connect (fp.get (),
						SIGNAL (error (const QString&)),
						this,
						SLOT (handleError (const QString&)));
				fp->Start ();
				return IFindProxy_ptr (fp);
			}

			void Plugin::handleError (const QString& msg)
			{
				qWarning () << Q_FUNC_INFO << sender () << msg;
				QMessageBox::critical (0,
						tr ("LeechCraft"),
						msg);
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_vgrabber, LeechCraft::Plugins::vGrabber::Plugin);

