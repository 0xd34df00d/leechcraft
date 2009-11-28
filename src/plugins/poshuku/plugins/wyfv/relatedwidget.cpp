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

#include "relatedwidget.h"
#include <algorithm>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsLinearLayout>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QtDebug>
#include "relateditem.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			namespace Plugins
			{
				namespace WYFV
				{
					RelatedWidget::RelatedWidget (QWidget *parent)
					: QGraphicsView (parent)
					, Scene_ (new QGraphicsScene)
					, Layout_ (new QGraphicsLinearLayout)
					, Shown_ (false)
					, OpacityAnimation_ (new QPropertyAnimation (this, "windowOpacity"))
					{
						connect (OpacityAnimation_,
								SIGNAL (finished ()),
								this,
								SLOT (handleAnimationFinished ()));

						setWindowTitle (tr ("Related videos"));
						setWindowFlags (Qt::WindowStaysOnTopHint);

						QGraphicsWidget *form = new QGraphicsWidget;
						form->setLayout (Layout_);
						Scene_->addItem (form);
					}

					RelatedWidget::~RelatedWidget ()
					{
						delete Layout_;
					}

					namespace
					{
						struct Orderer
						{
							bool operator() (const Related& r1, const Related& r2)
							{
								return r1.Rating_ >= r2.Rating_;
							}
						};
					};

					void RelatedWidget::SetRelated (QList<Related> related)
					{
						if (!related.size ())
							return;

						RelatedItems_.clear ();

						while (Layout_->count ())
						{
							QGraphicsLayoutItem *item = Layout_->itemAt (0);
							Layout_->removeAt (0);
							delete item;
						}

						std::sort (related.begin (), related.end (), Orderer ());

						int height = 100;

						Q_FOREACH (Related r, related)
						{
							RelatedItem *item = new RelatedItem ();
							item->SetRelated (r);
							if (item->height () < height)
								height = item->height ();

							Layout_->addItem (Scene_->addWidget (item));

							RelatedItems_ << item;
						}

						Layout_->invalidate ();

						setScene (Scene_);

						int width = QApplication::desktop ()->
							availableGeometry (this).width () * 2 / 3;
						resize (width, height * 2);
					}

					void RelatedWidget::ToggleVisibility ()
					{
						if (OpacityAnimation_->state () == QAbstractAnimation::Running)
						{
							qDebug () << "is running";
							OpacityAnimation_->stop ();
							setWindowOpacity (OpacityAnimation_->endValue ().value<qreal> ());
							handleAnimationFinished ();
						}

						int start = Shown_;
						if (!Shown_)
						{
							Q_FOREACH (QWidget *w, RelatedItems_)
								w->setWindowOpacity (0);
							setVisible (true);
						}
						OpacityAnimation_->setStartValue (static_cast<qreal> (start));
						OpacityAnimation_->setEndValue (static_cast<qreal> (1 - start));
						OpacityAnimation_->start ();
					}

					void RelatedWidget::handleAnimationFinished ()
					{
						Shown_ = !Shown_;
						setVisible (Shown_);
						if (Shown_)
						{
							QSequentialAnimationGroup *group = new QSequentialAnimationGroup;
							connect (group,
									SIGNAL (finished ()),
									group,
									SLOT (deleteLater ()));
							Q_FOREACH (QWidget *w, RelatedItems_)
							{
								QPropertyAnimation *a = new QPropertyAnimation (w, "windowOpacity");
								a->setStartValue (0.);
								a->setEndValue (1.);
								a->setDuration (100);
								group->addAnimation (a);
							}
							group->start ();
						}
					}
				};
			};
		};
	};
};

