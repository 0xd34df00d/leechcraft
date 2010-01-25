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

#include "kinotify.h"
#include <boost/bind.hpp>
#include <QIcon>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include "kineticnotification.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Kinotify
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;
				Proxy_->RegisterHook (HookSignature<HIDNotification>::Signature_t (
							boost::bind (&Plugin::HandleFinishedNotification,
								this,
								_1,
								_2,
								_3)));
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
			}

			QString Plugin::GetName () const
			{
				return "Kinotify";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Fancy Kinetic notifications.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon ();
			}

			QStringList Plugin::Provides () const
			{
				return QStringList ();
			}

			QStringList Plugin::Needs () const
			{
				return QStringList ();
			}

			QStringList Plugin::Uses () const
			{
				return QStringList ();
			}

			void Plugin::SetProvider (QObject*, const QString&)
			{
			}

			void Plugin::HandleFinishedNotification (IHookProxy_ptr proxy,
					Notification *n, bool show)
			{
				if (!show)
					return;

				if (n->Priority_ == Notification::PLog_)
					return;

				int timeout = Proxy_->GetSettingsManager ()->
					property ("FinishedDownloadMessageTimeout").toInt () * 1000;

				proxy->CancelDefault ();

				KineticNotification *kn = new KineticNotification (QString::number (rand ()),
						timeout);
				
				QString mi = "information";
				switch (n->Priority_)
				{
					case Notification::PWarning_:
						mi = "warning";
						break;
					case Notification::PCritical_:
						mi = "error";
					default:
						break;
				}

				QMap<int, QString> sizes = Proxy_->GetIconPath (mi);
				int size = 0;
				if (!sizes.contains (size))
					size = sizes.keys ().last ();
				QString path = sizes [size];
				kn->setMessage (n->Header_, n->Text_, path);
				kn->send ();
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_kinotify, LeechCraft::Plugins::Kinotify::Plugin);

