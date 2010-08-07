/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2009  Georg Rudoy
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

#include "seekthru.h"
#include <plugininterface/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "searcherslist.h"
#include "wizardgenerator.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			void SeekThru::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (Util::InstallTranslator ("seekthru"));

				Core::Instance ().SetProxy (proxy);

				connect (&Core::Instance (),
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)));
				connect (&Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
				connect (&Core::Instance (),
						SIGNAL (error (const QString&)),
						this,
						SLOT (handleError (const QString&)),
						Qt::QueuedConnection);
				connect (&Core::Instance (),
						SIGNAL (warning (const QString&)),
						this,
						SLOT (handleWarning (const QString&)),
						Qt::QueuedConnection);
				connect (&Core::Instance (),
						SIGNAL (categoriesChanged (const QStringList&, const QStringList&)),
						this,
						SIGNAL (categoriesChanged (const QStringList&, const QStringList&)));
				connect (&Core::Instance (),
						SIGNAL (newDeltasAvailable (const Sync::ChainID_t&)),
						this,
						SIGNAL (newDeltasAvailable (const Sync::ChainID_t&)));

				Core::Instance ().DoDelayedInit ();

				XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
						"seekthrusettings.xml");
				XmlSettingsDialog_->SetCustomWidget ("SearchersList", new SearchersList);

				Chains_ << Sync::ChainID_t ("osengines");
			}

			void SeekThru::SecondInit ()
			{
			}

			void SeekThru::Release ()
			{
				XmlSettingsDialog_.reset ();
			}

			QByteArray SeekThru::GetUniqueID () const
			{
				return "org.LeechCraft.SeekThru";
			}

			QString SeekThru::GetName () const
			{
				return "SeekThru";
			}

			QString SeekThru::GetInfo () const
			{
				return tr ("Search via OpenSearch-aware search providers.");
			}

			QIcon SeekThru::GetIcon () const
			{
				return QIcon (":/resources/images/seekthru.svg");
			}

			QStringList SeekThru::Provides () const
			{
				return QStringList ("search");
			}

			QStringList SeekThru::Needs () const
			{
				return QStringList ("http");
			}

			QStringList SeekThru::Uses () const
			{
				return QStringList ("webbrowser");
			}

			void SeekThru::SetProvider (QObject *object, const QString& feature)
			{
				Core::Instance ().SetProvider (object, feature);
			}

			QStringList SeekThru::GetCategories () const
			{
				return Core::Instance ().GetCategories ();
			}

			QList<IFindProxy_ptr> SeekThru::GetProxy (const LeechCraft::Request& r)
			{
				QList<IFindProxy_ptr> result;
				result << Core::Instance ().GetProxy (r);
				return result;
			}

			boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> SeekThru::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}

			bool SeekThru::CouldHandle (const LeechCraft::Entity& e) const
			{
				return Core::Instance ().CouldHandle (e);
			}

			void SeekThru::Handle (LeechCraft::Entity e)
			{
				Core::Instance ().Add (e.Entity_.toUrl ());
			}

			QList<QWizardPage*> SeekThru::GetWizardPages () const
			{
				std::auto_ptr<WizardGenerator> wg (new WizardGenerator);
				return wg->GetPages ();
			}

			Sync::ChainIDs_t SeekThru::AvailableChains () const
			{
				return Chains_;
			}

			Sync::Payloads_t SeekThru::GetAllDeltas (const Sync::ChainID_t& chainId) const
			{
				return Core::Instance ().GetAllDeltas (chainId);
			}

			Sync::Payloads_t SeekThru::GetNewDeltas (const Sync::ChainID_t& chainId) const
			{
				return Core::Instance ().GetNewDeltas (chainId);
			}

			void SeekThru::PurgeNewDeltas (const Sync::ChainID_t& chainId)
			{
				Core::Instance ().PurgeNewDeltas (chainId);
			}

			void SeekThru::ApplyDeltas (const Sync::Payloads_t& payloads,
					const Sync::ChainID_t& chainId)
			{
				Core::Instance ().ApplyDeltas (payloads, chainId);
			}

			void SeekThru::handleError (const QString& error)
			{
				emit gotEntity (Util::MakeNotification ("SeekThru", error, PCritical_));
			}

			void SeekThru::handleWarning (const QString& error)
			{
				emit gotEntity (Util::MakeNotification ("SeekThru", error, PWarning_));
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_seekthru, LeechCraft::Plugins::SeekThru::SeekThru);
