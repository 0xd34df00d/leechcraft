/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010  Georg Rudoy
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

#include "azoth.h"
#include <QIcon>
#include <QDockWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include "core.h"
#include "mainwidget.h"
#include "chattabsmanager.h"
#include "chattab.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Azoth
		{
			void Plugin::Init (ICoreProxy_ptr proxy)
			{
				ChatTab::SetParentMultiTabs (this);

				Core::Instance ().SetProxy (proxy);

				QMainWindow *mainWin = proxy->GetMainWindow ();
				QDockWidget *dw = new QDockWidget (mainWin);
				MW_ = new MainWidget ();
				dw->setWidget (MW_);
				dw->setWindowTitle ("Azoth");

				mainWin->addDockWidget (Qt::RightDockWidgetArea, dw);

				connect (&Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));

				handleAccountCreatorActionsAdded (Core::Instance ().GetAccountCreatorActions ());
				connect (&Core::Instance (),
						SIGNAL (accountCreatorActionsAdded (const QList<QAction*>&)),
						this,
						SLOT (handleAccountCreatorActionsAdded (const QList<QAction*>&)));

				handleMUCJoinActionsAdded (Core::Instance ().GetMUCJoinActions ());
				connect (&Core::Instance (),
						SIGNAL (mucJoinActionsAdded (const QList<QAction*>&)),
						this,
						SLOT (handleMUCJoinActionsAdded (const QList<QAction*>&)));

				connect (Core::Instance ().GetChatTabsManager (),
						SIGNAL (addNewTab (const QString&, QWidget*)),
						this,
						SIGNAL (addNewTab (const QString&, QWidget*)));
				connect (Core::Instance ().GetChatTabsManager (),
						SIGNAL (removeTab (QWidget*)),
						this,
						SIGNAL (removeTab (QWidget*)));
				connect (Core::Instance ().GetChatTabsManager (),
						SIGNAL (raiseTab (QWidget*)),
						this,
						SIGNAL (raiseTab (QWidget*)));
			}

			void Plugin::SecondInit ()
			{
			}

			void Plugin::Release ()
			{
			}

			QByteArray Plugin::GetUniqueID () const
			{
				return "org.LeechCraft.Azoth";
			}

			QString Plugin::GetName () const
			{
				return "Azoth";
			}

			QString Plugin::GetInfo () const
			{
				return tr ("Extensible IM client for LeechCraft.");
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

			QSet<QByteArray> Plugin::GetExpectedPluginClasses () const
			{
				return Core::Instance ().GetExpectedPluginClasses ();
			}

			void Plugin::AddPlugin (QObject *object)
			{
				Core::Instance ().AddPlugin (object);
			}

			void Plugin::handleAccountCreatorActionsAdded (const QList<QAction*>& actions)
			{
				MW_->AddAccountCreators (actions);
			}

			void Plugin::handleMUCJoinActionsAdded (const QList<QAction*>& actions)
			{
				MW_->AddMUCJoiners (actions);
			}

			void Plugin::newTabRequested ()
			{
			}
		};
	};
};

Q_EXPORT_PLUGIN2 (leechcraft_azoth, LeechCraft::Plugins::Azoth::Plugin);

