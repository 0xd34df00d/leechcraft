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

#include "tabppwidget.h"
#include <QIcon>
#include <QApplication>
#include <QDesktopWidget>
#include <QMainWindow>
#include <QTimer>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "tabsfiltermodel.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace TabPP
		{
			TabPPWidget::TabPPWidget (const QString& title, QWidget *parent)
			: QDockWidget (title, parent)
			, ShouldFloat_ (false)
			, TabsFilterModel_ (new TabsFilterModel (this))
			{
				Ui_.setupUi (this);

				GetActivatorAction ()->setShortcut (tr ("F9"));

				connect (GetActivatorAction (),
						SIGNAL (hovered ()),
						this,
						SLOT (handleActivatorHovered ()));
				connect (GetActivatorAction (),
						SIGNAL (triggered ()),
						this,
						SLOT (handleFirstTriggered ()));

				TabsFilterModel_->setSourceModel (Core::Instance ().GetModel ());
				TabsFilterModel_->setDynamicSortFilter (true);
				TabsFilterModel_->setFilterCaseSensitivity (Qt::CaseInsensitive);
				Ui_.View_->setModel (TabsFilterModel_);

				connect (Ui_.FilterLine_,
						SIGNAL (textChanged (const QString&)),
						TabsFilterModel_,
						SLOT (setFilterFixedString (const QString&)));

				connect (Core::Instance ().GetModel (),
						SIGNAL (rowsInserted (const QModelIndex&,
								int, int)),
						Ui_.View_,
						SLOT (expandAll ()));
				connect (Core::Instance ().GetModel (),
						SIGNAL (rowsRemoved (const QModelIndex&,
								int, int)),
						Ui_.View_,
						SLOT (expandAll ()));

				connect (Ui_.View_,
						SIGNAL (activated (const QModelIndex&)),
						this,
						SLOT (selected (const QModelIndex&)));

				connect (this,
						SIGNAL (dockLocationChanged (Qt::DockWidgetArea)),
						this,
						SLOT (handleDockLocationChanged (Qt::DockWidgetArea)));
				connect (this,
						SIGNAL (topLevelChanged (bool)),
						this,
						SLOT (handleTopLevelChanged (bool)));
				connect (this,
						SIGNAL (visibilityChanged (bool)),
						this,
						SLOT (handleVisibilityChanged (bool)));

				QMainWindow *mw = Core::Instance ().GetProxy ()->GetMainWindow ();
				int area = XmlSettingsManager::Instance ()
					.Property ("DockArea", Qt::NoDockWidgetArea).toInt ();
				switch (area)
				{
					case Qt::LeftDockWidgetArea:
					case Qt::RightDockWidgetArea:
					case Qt::TopDockWidgetArea:
					case Qt::BottomDockWidgetArea:
						mw->addDockWidget (static_cast<Qt::DockWidgetArea> (area),
								this);
						break;
					default:
						mw->addDockWidget (Qt::LeftDockWidgetArea,
								this);
						ShouldFloat_ = true;
						break;
				}

				bool visible = XmlSettingsManager::Instance ()
					.Property ("Visible", false).toBool ();
				QTimer::singleShot (0,
						this,
						visible ?
						SLOT (show ()) :
						SLOT (hide ()));
			}

			void TabPPWidget::Release ()
			{
				disconnect (this,
						0,
						this,
						0);
			}

			QTreeView* TabPPWidget::GetView () const
			{
				return Ui_.View_;
			}

			QAction* TabPPWidget::GetActivatorAction () const
			{
				return toggleViewAction ();
			}

			void TabPPWidget::handleActivatorHovered ()
			{
				if (!XmlSettingsManager::Instance ()
						.property ("ShowOnHover").toBool ())
					return;

				if (isVisible ())
					return;

				if (ShouldFloat_)
				{
					ShouldFloat_ = false;
					setFloating (true);
				}
				show ();
			}

			void TabPPWidget::handleFirstTriggered ()
			{
				if (ShouldFloat_)
				{
					ShouldFloat_ = false;
					setFloating (true);
					show ();
				}
				disconnect (GetActivatorAction (),
						SIGNAL (triggered ()),
						this,
						SLOT (handleFirstTriggered ()));
			}

			void TabPPWidget::selected (const QModelIndex& index)
			{
				Core::Instance ().HandleSelected (TabsFilterModel_->mapToSource (index));
			}

			void TabPPWidget::handleDockLocationChanged (Qt::DockWidgetArea area)
			{
				XmlSettingsManager::Instance ().setProperty ("DockArea", area);
			}

			void TabPPWidget::handleTopLevelChanged (bool top)
			{
				if (top)
					XmlSettingsManager::Instance ().setProperty ("DockArea",
							Qt::NoDockWidgetArea);
			}

			void TabPPWidget::handleVisibilityChanged (bool visible)
			{
				if (visible && ShouldFloat_)
				{
					ShouldFloat_ = false;
					setFloating (true);
				}

				XmlSettingsManager::Instance ().setProperty ("Visible", visible);

				if (visible &&
						XmlSettingsManager::Instance ()
							.property ("FocusOnShow").toBool ())
					Ui_.View_->setFocus ();
			}
		};
	};
};

