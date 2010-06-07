/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
 * * This program is free software: you can redistribute it and/or modify
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

#include "auscrie.h"
#include <QMainWindow>
#include <QIcon>
#include <QTimer>
#include <QBuffer>
#include <QDir>
#include <QFileDialog>
#include <xmlsettingsdialog/basesettingsmanager.h>
#include <plugininterface/util.h>
#include "shooterdialog.h"
#include "poster.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Auscrie
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;

				Translator_.reset (Util::InstallTranslator ("auscrie"));

				Dialog_ = new ShooterDialog (Proxy_->GetMainWindow ());

				ShotAction_ = new QAction (GetIcon (),
						tr ("Make a screenshot"),
						this);
				connect (ShotAction_,
						SIGNAL (triggered ()),
						this,
						SLOT (makeScreenshot ()));
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
			}

			QString Plugin::GetName () const
			{
				return "Auscrie";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Simple auto screenshoter.");
			}

			QIcon Plugin::GetIcon () const
			{
				return QIcon (":/resources/images/auscrie.svg");
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

			QList<QAction*> Plugin::GetActions () const
			{
				QList<QAction*> result;
				result << ShotAction_;
				return result;
			}

			void Plugin::makeScreenshot ()
			{
				if (Dialog_->exec () != QDialog::Accepted)
					return;

				ShotAction_->setEnabled (false);
				QTimer::singleShot (Dialog_->GetTimeout () * 1000,
						this,
						SLOT (shoot ()));
			}

			void Plugin::shoot ()
			{
				ShotAction_->setEnabled (true);

				QWidget *mw = Proxy_->GetMainWindow ();
				QPixmap pm = QPixmap::grabWidget (mw);

				int quality = Dialog_->GetQuality ();

				switch (Dialog_->GetAction ())
				{
					case ShooterDialog::ASave:
						{
							QString path = Proxy_->GetSettingsManager ()->
								Property ("PluginsStorage/Auscrie/SavePath",
										QDir::currentPath () + "01." + Dialog_->GetFormat ())
								.toString ();

							QString filename = QFileDialog::getSaveFileName (mw,
									tr ("Save as"),
									path,
									tr ("%1 files (*.%1);;All files (*.*)")
										.arg (Dialog_->GetFormat ()));

							if (!filename.isEmpty ())
							{
								pm.save (filename,
										qPrintable (Dialog_->GetFormat ()),
										quality);
								Proxy_->GetSettingsManager ()->
									setProperty ("PluginsStorage/Auscrie/SavePath",
											filename);
							}
						}
						break;
					case ShooterDialog::AUpload:
						{
							QByteArray bytes;
							QBuffer buf (&bytes);
							buf.open (QIODevice::ReadWrite);
							if (!pm.save (&buf,
										qPrintable (Dialog_->GetFormat ()),
										quality))
								qWarning () << Q_FUNC_INFO
									<< "save failed"
									<< qPrintable (Dialog_->GetFormat ())
									<< quality;
							Post (bytes);
							break;
						}
				}
			}

			void Plugin::Post (const QByteArray& data)
			{
				Poster *p = new Poster (data,
						Dialog_->GetFormat (),
						Proxy_->GetNetworkAccessManager ());
				connect (p,
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_auscrie, LeechCraft::Plugins::Auscrie::Plugin);

