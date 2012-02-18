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

#include "vgrabber.h"
#include <QIcon>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/itagsmanager.h>
#include "audiofindproxy.h"
#include "videofindproxy.h"
#include "xmlsettingsmanager.h"
#include "categoriesselector.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			void vGrabber::Init (ICoreProxy_ptr proxy)
			{
				Proxy_ = proxy;

				Translator_.reset (Util::InstallTranslator ("vgrabber"));

				SettingsDialog_.reset (new Util::XmlSettingsDialog ());
				SettingsDialog_->RegisterObject (XmlSettingsManager::Instance (),
						"vgrabbersettings.xml");

				Audio_ = new CategoriesSelector (FindProxy::FPTAudio,
						this);
				Video_ = new CategoriesSelector (FindProxy::FPTVideo,
						this);
				connect (Audio_,
						SIGNAL (goingToAccept (const QStringList&,
								const QStringList&)),
						this,
						SLOT (handleCategoriesGoingToChange (const QStringList&,
								const QStringList&)));
				connect (Video_,
						SIGNAL (goingToAccept (const QStringList&,
								const QStringList&)),
						this,
						SLOT (handleCategoriesGoingToChange (const QStringList&,
								const QStringList&)));

				SettingsDialog_->SetCustomWidget ("AudioCategoriesToUse", Audio_);
				SettingsDialog_->SetCustomWidget ("VideoCategoriesToUse", Video_);
			}

			void vGrabber::SecondInit ()
			{
			}

			void vGrabber::Release ()
			{
				delete Audio_;
				delete Video_;
				Translator_.reset ();
			}

			QByteArray vGrabber::GetUniqueID () const
			{
				return "org.LeechCraft.vGrabber";
			}

			QString vGrabber::GetName () const
			{
				return "vGrabber";
			}

			QString vGrabber::GetInfo () const
			{
				return tr ("vkontakte.ru audio/video grabber.");
			}

			QIcon vGrabber::GetIcon () const
			{
				return QIcon (":/resources/images/vgrabber.svg");
			}

			QStringList vGrabber::Needs () const
			{
				return QStringList ("http");
			}

			QStringList vGrabber::GetCategories () const
			{
				QStringList result;
				result += Audio_->GetHRCategories ();
				result += Video_->GetHRCategories ();
				return result;
			}

			QList<IFindProxy_ptr> vGrabber::GetProxy (const Request& req)
			{
				QList<FindProxy_ptr> preresult;
				if (Audio_->GetHRCategories ().contains (req.Category_))
					preresult << FindProxy_ptr (new AudioFindProxy (req, Audio_));

				if (Video_->GetHRCategories ().contains (req.Category_))
					preresult << FindProxy_ptr (new VideoFindProxy (req, Video_));

				QList<IFindProxy_ptr> result;
				Q_FOREACH (FindProxy_ptr fp, preresult)
				{
					connect (fp.get (),
							SIGNAL (delegateEntity (const LeechCraft::Entity&,
									int*, QObject**)),
							this,
							SIGNAL (delegateEntity (const LeechCraft::Entity&,
									int*, QObject**)));
					connect (fp.get (),
							SIGNAL (gotEntity (const LeechCraft::Entity&)),
							this,
							SIGNAL (gotEntity (const LeechCraft::Entity&)));
					connect (fp.get (),
							SIGNAL (error (const QString&)),
							this,
							SLOT (handleError (const QString&)));

					fp->Start ();

					result << IFindProxy_ptr (fp);
				}
				return result;
			}

			ICoreProxy_ptr vGrabber::GetProxy () const
			{
				return Proxy_;
			}

			std::shared_ptr<Util::XmlSettingsDialog> vGrabber::GetSettingsDialog () const
			{
				return SettingsDialog_;
			}

			void vGrabber::handleError (const QString& msg)
			{
				qWarning () << Q_FUNC_INFO << sender () << msg;
				emit gotEntity (Util::MakeNotification ("vGrabber", msg, PWarning_));
			}

			void vGrabber::handleCategoriesGoingToChange (const QStringList& added,
					const QStringList& removed)
			{
				QStringList hrAdded, hrRemoved;
				Q_FOREACH (QString a, added)
					hrAdded << Proxy_->GetTagsManager ()->GetTag (a);
				Q_FOREACH (QString r, removed)
					hrRemoved << Proxy_->GetTagsManager ()->GetTag (r);

				emit categoriesChanged (hrAdded, hrRemoved);
			}
		};
	};
};

LC_EXPORT_PLUGIN (leechcraft_vgrabber, LeechCraft::Plugins::vGrabber::vGrabber);

