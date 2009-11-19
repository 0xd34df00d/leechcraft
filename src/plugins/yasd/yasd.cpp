/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2009  Georg Rudoy
 *				 2009  Snark
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

#include "yasd.h"
#include <QIcon>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace YASD
		{
			void Plugin::Init (ICoreProxy_ptr)
			{
				Translator_.reset (Util::InstallTranslator ("yasd"));
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
				Translator_.reset ();
			}

			QString Plugin::GetName () const
			{
				return "YASD";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Yet Another Site Downloader");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon (":/resources/images/yasd.svg");
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
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_yasd, LeechCraft::Plugins::YASD::Plugin);

