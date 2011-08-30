/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2011 Minh Ngo
 * Copyright (C) 2006-2011  Georg Rudoy
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

#include "potorchu.h"
#include <QIcon>
#include <QUrl>
#include <util/util.h>

#include "xmlsettingsmanager.h"

namespace LeechCraft
{
	namespace Potorchu
	{
			void Potorchu::Init (ICoreProxy_ptr proxy)
			{
				XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
						"potorchusettings.xml");
				Proxy_ = proxy;
				PotorchuWidget::SetParentMultiTabs (this);
				TabClassInfo tabClass =
				{
					"Potorchu",
					"Potorchu",
					GetInfo (),
					GetIcon (),
					50,
					TabFeatures (TFOpenableByRequest | TFByDefault)
				};
				TabClasses_ << tabClass;
			}

			void Potorchu::SecondInit ()
			{
			}

			QByteArray Potorchu::GetUniqueID () const
			{
				return "org.LeechCraft.Potorchu";
			}

			void Potorchu::Release ()
			{
			}

			QString Potorchu::GetName () const
			{
				return "Potorchu";
			}

			QString Potorchu::GetInfo () const
			{
				return tr ("");
			}

			QIcon Potorchu::GetIcon () const
			{
				return QIcon ();
			}

			QStringList Potorchu::Provides () const
			{
				return QStringList ();
			}

			QStringList Potorchu::Needs () const
			{
				return QStringList ();
			}

			QStringList Potorchu::Uses () const
			{
				return QStringList ();
			}

			void Potorchu::SetProvider (QObject*, const QString&)
			{
			}
			
			TabClasses_t Potorchu::GetTabClasses () const
			{
				return TabClasses_;
			}
			
			void Potorchu::TabOpenRequested (const QByteArray& tabClass)
			{
				if (tabClass == "Potorchu")
					createTab ();
				else
				{
					qWarning () << Q_FUNC_INFO
							<< "unknown tab class"
							<< tabClass;
				}
			}
			
			void Potorchu::handleNeedToClose ()
			{
				PotorchuWidget *w = qobject_cast<PotorchuWidget*> (sender ());
				if (!w)
				{
					qWarning () << Q_FUNC_INFO
						<< "not a PotorchuWidget*"
						<< sender ();
					return;
				}
				emit removeTab (w);
				Others_.removeAll (w);
				w->deleteLater ();
			}

			PotorchuWidget *Potorchu::createTab ()
			{
				PotorchuWidget *w = new PotorchuWidget ();
				w->Init (Proxy_);
				connect (w,
						SIGNAL (needToClose ()),
						this,
						SLOT (handleNeedToClose ()));
					
				Others_ << w;
				emit addNewTab (tr ("Potorchu"), w);
				emit changeTabIcon (w, QIcon ());
				emit raiseTab (w);
				return w;
			}
			
			Util::XmlSettingsDialog_ptr Potorchu::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}

			
			EntityTestHandleResult Potorchu::CouldHandle (const Entity& entity) const
			{
				bool stat;
				if (entity.Mime_ == "application/ogg")
                                        stat = true;
                                if (entity.Mime_.startsWith ("audio/"))
                                        stat = true;
                                if (entity.Mime_.startsWith ("video/"))
                                        stat = true;
                                else
					stat = false;
				return stat ?
						EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
						EntityTestHandleResult ();
			}
			
			void Potorchu::Handle (Entity entity)
			{
				const QString& dest = entity.Entity_.toString ();
				PotorchuWidget *w = createTab ();
				w->handleOpenMediaContent (dest);
			}
	}
}

Q_EXPORT_PLUGIN2 (leechcraft_potorchu, LeechCraft::Potorchu::Potorchu);

