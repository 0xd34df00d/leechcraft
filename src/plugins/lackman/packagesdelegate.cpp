/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2010  Georg Rudoy
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

#include "packagesdelegate.h"
#include <QPainter>
#include <QApplication>
#include <QTreeView>
#include <QAction>
#include <QToolButton>
#include <QTimer>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMainWindow>
#include <QAbstractProxyModel>
#include <QtDebug>
#include "packagesmodel.h"
#include "core.h"
#include "pendingmanager.h"
#include "delegatebuttongroup.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace LackMan
		{
			const int PackagesDelegate::CPadding = 7;
			const int PackagesDelegate::CIconSize = 32;
			const int PackagesDelegate::CActionsSize = 32;
			const int PackagesDelegate::CTitleSizeDelta = 2;
			const int PackagesDelegate::CNumLines = 3;

			PackagesDelegate::PackagesDelegate (QTreeView *parent)
			: QStyledItemDelegate (parent)
			, Viewport_ (parent->viewport ())
			, Model_ (parent->model ())
			{
				qDebug () << Q_FUNC_INFO << parent->model ();
				connect (parent->verticalScrollBar (),
						SIGNAL (valueChanged (int)),
						this,
						SLOT (invalidateWidgetPositions ()),
						Qt::QueuedConnection);
				connect (Model_,
						SIGNAL (rowsRemoved (const QModelIndex&, int, int)),
						this,
						SLOT (hideOverflousActions (const QModelIndex&, int, int)),
						Qt::QueuedConnection);
				connect (&Core::Instance (),
						SIGNAL (packageRowActionFinished (int)),
						this,
						SLOT (handleRowActionFinished (int)),
						Qt::QueuedConnection);

				connect (Core::Instance ().GetPendingManager (),
						SIGNAL (packageUpdateToggled (int, bool)),
						this,
						SLOT (handlePackageUpdateToggled (int, bool)));
			}

			void PackagesDelegate::paint (QPainter *painter,
					const QStyleOptionViewItem& option, const QModelIndex& index) const
			{
				QStyleOptionViewItemV4 opt (option);
				QStyle *style = opt.widget ?
						opt.widget->style () :
						QApplication::style ();
				QFontMetrics fontMetrics = opt.widget ?
						opt.widget->fontMetrics () :
						QApplication::fontMetrics ();

				const QRect& r = option.rect;
				bool ltr = (painter->layoutDirection () == Qt::LeftToRight);
				bool selected = option.state & QStyle::State_Selected;

				QString title = index.data (Qt::DisplayRole).toString ();
				QString shortDescr = index.data (PackagesModel::PMRShortDescription).toString ();
				QString version = index.data (PackagesModel::PMRVersion).toString ();
				QStringList tags = index.data (PackagesModel::PMRTags).toStringList ();
				QColor fgColor = selected ?
						option.palette.color (QPalette::HighlightedText) :
						option.palette.color (QPalette::Text);

				QIcon icon = index.data (Qt::DecorationRole).value<QIcon> ();

				QStyleOptionViewItem titleOption (option);
				titleOption.font.setBold (true);
				titleOption.font.setPointSize (titleOption.font.pointSize () + CTitleSizeDelta);

				QPixmap pixmap (option.rect.size ());
				pixmap.fill (Qt::transparent);
				QPainter p (&pixmap);
				p.translate (-option.rect.topLeft ());

				if (selected ||
						(option.state & QStyle::State_MouseOver))
					style->drawPrimitive (QStyle::PE_PanelItemViewItem, &opt, &p, opt.widget);

				int textShift = 2 * CPadding + CIconSize;
				int leftPos = r.left () + (ltr ? textShift : 0);
				int textWidth = r.width () - textShift - CPadding;
				int shiftFromTop = r.top () + CPadding;
				int textHeight = TextHeight (option);

				p.setPen (fgColor);
				p.setFont (titleOption.font);
				p.drawText (leftPos, shiftFromTop,
						textWidth, TitleHeight (option),
						Qt::AlignBottom | Qt::AlignLeft, title);
				int titleWidth = QFontMetrics (titleOption.font).width (title) + CPadding;

				QFont tagsFont = option.font;
				tagsFont.setItalic (true);

				p.setFont (tagsFont);
				p.drawText (leftPos + titleWidth, shiftFromTop,
						textWidth - titleWidth, textHeight,
						Qt::AlignBottom | Qt::AlignLeft, version);
				titleWidth += QFontMetrics (titleOption.font).width (version) + CPadding;

				QString tagsString = QFontMetrics (tagsFont).elidedText (tags.join ("; "),
								Qt::ElideMiddle, textWidth - titleWidth);

				p.setFont (tagsFont);
				p.drawText (leftPos + titleWidth, shiftFromTop,
						textWidth - titleWidth, textHeight,
						Qt::AlignBottom | Qt::AlignRight, tagsString);

				shiftFromTop += TitleHeight (option);

				p.setFont (option.font);
				shortDescr = fontMetrics.elidedText (shortDescr,
						option.textElideMode, textWidth);
				p.drawText (leftPos, shiftFromTop,
						textWidth, textHeight,
						Qt::AlignTop | Qt::AlignLeft, shortDescr);

				shiftFromTop += textHeight;

				style->drawPrimitive (QStyle::PE_FrameGroupBox, &option, &p);

				p.end ();

				painter->drawPixmap (option.rect, pixmap);

				icon.paint (painter,
						ltr ?
								r.left () + CPadding :
								r.left () + r.width () - CPadding - CIconSize,
						r.top () + CPadding,
						CIconSize, CIconSize,
						Qt::AlignCenter, QIcon::Normal);

				QWidget *layoutWidget = GetLayout (index);
				QPoint actionPos (leftPos, shiftFromTop);
				if (layoutWidget->pos () != actionPos)
					layoutWidget->move (actionPos);
				if (!layoutWidget->isVisible ())
					layoutWidget->show ();

				shiftFromTop += CActionsSize + CPadding;

				if (selected)
				{
					PrepareSelectableBrowser ();
					SelectableBrowser_->SetHtml (index.data (PackagesModel::PMRLongDescription).toString ());

					QPoint browserPos (leftPos, shiftFromTop);
					if (SelectableBrowser_->pos () != browserPos)
						SelectableBrowser_->move (browserPos);

					QSize browserSize (textWidth, CurrentInfoHeight (option));
					if (SelectableBrowser_->size () != browserSize)
						SelectableBrowser_->resize (browserSize);

					if (!SelectableBrowser_->isVisible ())
						SelectableBrowser_->show ();
				}
			}

			QSize PackagesDelegate::sizeHint (const QStyleOptionViewItem& option,
					const QModelIndex& index) const
			{
				QSize result = index.data (Qt::SizeHintRole).toSize ();

				// One padding from the top, one from the bottom, and
				// one between webview and install/remove actions (if
				// selected).
				result.rheight () = TitleHeight (option) +
						TextHeight (option) +
						CActionsSize + CPadding * 2;
				if (index == CurrentSelection_)
				{
					result.rheight () += CurrentInfoHeight (option);
					result.rheight () += CPadding;
				}

				return result;
			}

			int PackagesDelegate::TitleHeight (const QStyleOptionViewItem& option) const
			{
				QFont boldFont = option.font;

				boldFont.setBold (true);
				boldFont.setPointSize (boldFont.pointSize () + CTitleSizeDelta);

				return QFontInfo (boldFont).pixelSize () + CPadding;
			}

			int PackagesDelegate::TextHeight (const QStyleOptionViewItem& option) const
			{
				return QFontInfo (option.font).pixelSize () + CPadding;
			}

			int PackagesDelegate::CurrentInfoHeight (const QStyleOptionViewItem& option) const
			{
				return 140;
			}

			void PackagesDelegate::PrepareSelectableBrowser () const
			{
				if (SelectableBrowser_)
					return;

				SelectableBrowser_ = new Util::SelectableBrowser ();
				QList<IWebBrowser*> browsers = Core::Instance ().GetProxy ()->
						GetPluginsManager ()->GetAllCastableTo<IWebBrowser*> ();
				if (browsers.size ())
					SelectableBrowser_->Construct (browsers.at (0));
				SelectableBrowser_->setParent (Viewport_);
				SelectableBrowser_->SetNavBarVisible (false);
				SelectableBrowser_->SetEverythingElseVisible (false);
			}

			QToolButton* PackagesDelegate::GetInstallRemove (const QModelIndex& index) const
			{
				int row = index.row ();
				if (!Row2InstallRemove_.contains (row))
				{
					QAction *action = new QAction (Viewport_);
					action->setCheckable (true);
					action->setProperty ("Role", "InstallRemove");
					connect (action,
							SIGNAL (triggered ()),
							this,
							SLOT (handleAction ()));

					QToolButton *toolButton = new QToolButton ();
					toolButton->resize (CActionsSize, CActionsSize);
					toolButton->setDefaultAction (action);
					Row2InstallRemove_ [row] = toolButton;
				}

				QToolButton *button = Row2InstallRemove_ [row];

				bool installed = index.data (PackagesModel::PMRInstalled).toBool ();
				QString label;
				QString iconName;
				if (installed)
				{
					label = tr ("Remove");
					iconName = "remove";
				}
				else
				{
					label = tr ("Install");
					iconName = "addjob";
				}

				QAction *action = button->defaultAction ();
				WasInstalled_ [index] = installed;

				action->setText (label);
				action->setIcon (Core::Instance ().GetProxy ()->GetIcon (iconName));
				action->setData (index.data (PackagesModel::PMRPackageID));
				action->setProperty ("Installed", index.data (PackagesModel::PMRInstalled));

				return button;
			}

			QToolButton* PackagesDelegate::GetUpdate (const QModelIndex& index) const
			{
				int row = index.row ();
				if (!Row2Update_.contains (row))
				{
					QAction *action = new QAction (Core::Instance ()
								.GetProxy ()->GetIcon ("update"),
							tr ("Update"),
							Viewport_);
					action->setCheckable (true);
					action->setProperty ("Role", "Update");
					connect (action,
							SIGNAL (triggered ()),
							this,
							SLOT (handleAction ()));

					QToolButton *toolButton = new QToolButton ();
					toolButton->resize (CActionsSize, CActionsSize);
					toolButton->setDefaultAction (action);
					Row2Update_ [row] = toolButton;
				}

				QToolButton *button = Row2Update_ [row];
				QAction *action = button->defaultAction ();

				bool upgradable = index.data (PackagesModel::PMRUpgradable).toBool ();
				action->setEnabled (upgradable);
				action->setData (index.data (PackagesModel::PMRPackageID));

				WasUpgradable_ [index] = upgradable;

				return button;
			}

			QWidget* PackagesDelegate::GetLayout (const QModelIndex& index) const
			{
				if (!Row2Layout_.contains (index.row ()))
				{
					QToolButton *instRem = GetInstallRemove (index);
					QToolButton *update = GetUpdate (index);

					DelegateButtonGroup *group = new DelegateButtonGroup (Viewport_);
					group->AddButton (instRem);
					group->AddButton (update);

					QWidget *result = new QWidget (Viewport_);

					QHBoxLayout *layout = new QHBoxLayout (result);
					layout->addWidget (instRem);
					layout->addWidget (update);

					result->setLayout (layout);

					Row2Layout_ [index.row ()] = result;
				}
				else
				{
					bool isInstalled = index.data (PackagesModel::PMRInstalled).toBool ();
					bool isUpgradable = index.data (PackagesModel::PMRUpgradable).toBool ();

					if (isInstalled != WasInstalled_ [index])
						GetInstallRemove (index);

					if (isUpgradable != WasUpgradable_ [index])
						GetUpdate (index);
				}

				return Row2Layout_ [index.row ()];
			}

			void PackagesDelegate::handleRowChanged (const QModelIndex& current, const QModelIndex& previous)
			{
				CurrentSelection_ = current;

				if (SelectableBrowser_)
					SelectableBrowser_->hide ();

				emit sizeHintChanged (previous);
				emit sizeHintChanged (current);
			}

			void PackagesDelegate::invalidateWidgetPositions ()
			{
				if (SelectableBrowser_)
					SelectableBrowser_->hide ();
				QTreeView *view = qobject_cast<QTreeView*> (parent ());
				QAbstractItemModel *model = view->model ();
				for (int i = 0, rows = model->rowCount ();
						i < rows; ++i)
					emit sizeHintChanged (model->index (i, 0));
			}

			void PackagesDelegate::hideOverflousActions (const QModelIndex&,
					int, int)
			{
				int rowCount = Model_->rowCount ();
				int thisCount = Row2Layout_.size ();
				for (int i = rowCount; i < thisCount; ++i)
					Row2Layout_ [i]->hide ();
			}

			void PackagesDelegate::handleAction ()
			{
				QAction *sAction = qobject_cast<QAction*> (sender ());
				if (!sAction)
				{
					qWarning () << Q_FUNC_INFO
							<< "sender is not an action"
							<< sender ();
					return;
				}

				int packageID = sAction->data ().toInt ();
				QString role = sAction->property ("Role").toString ();
				bool checked = sAction->isChecked ();

				try
				{
					if (role == "InstallRemove")
					{
						bool installed = sAction->property ("Installed").toBool ();
						Core::Instance ().GetPendingManager ()->ToggleInstallRemove (packageID, checked, installed);
					}
					else if (role == "Update")
						Core::Instance ().GetPendingManager ()->ToggleUpdate (packageID, checked);
					else
						qWarning () << Q_FUNC_INFO
								<< "unknown role"
								<< role;
				}
				catch (const std::exception& e)
				{
					qWarning () << Q_FUNC_INFO
							<< e.what ();
					QMessageBox::critical (Core::Instance ().GetProxy ()->GetMainWindow (),
							"LeechCraft",
							tr ("Unable to mark package, reverting.") + "<br />" + QString::fromUtf8 (e.what ()));

					sAction->setChecked (false);
				}
			}

			namespace
			{
				QAbstractItemModel* GetRealModel (QAbstractItemModel *source,
							QList<QAbstractProxyModel*> *proxyChain = 0)
				{
					QAbstractProxyModel *proxy = 0;
					while ((proxy = qobject_cast<QAbstractProxyModel*> (source)))
					{
						if (proxyChain)
							proxyChain->push_front (proxy);
						source = proxy->sourceModel ();
					}
					return source;
				}
			}

			void PackagesDelegate::handleRowActionFinished (int row)
			{
				// Front — deepest proxy, back — outermost.
				QList<QAbstractProxyModel*> proxyChain;
				QAbstractItemModel *realModel = GetRealModel (Model_, &proxyChain);

				QModelIndex index = realModel->index (row, 0);

				Q_FOREACH (QAbstractProxyModel *pm, proxyChain)
					index = pm->mapFromSource (index);

				row = index.row ();

				if (Row2InstallRemove_.contains (row))
				{
					Row2InstallRemove_ [row]->setChecked (false);
					Row2InstallRemove_ [row]->defaultAction ()->setChecked (false);
					Row2InstallRemove_ [row]->setChecked (false);
				}
				if (Row2Update_.contains (row))
					Row2Update_ [row]->defaultAction ()->setChecked (false);
			}

			void PackagesDelegate::handlePackageUpdateToggled (int id, bool enabled)
			{
				PackagesModel *realModel =
						qobject_cast<PackagesModel*> (GetRealModel (Model_));
				int row = realModel->GetRow (id);
				if (Row2Update_.contains (row))
					Row2Update_ [row]->defaultAction ()->setChecked (enabled);
			}
		}
	}
}
