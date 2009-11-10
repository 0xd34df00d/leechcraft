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

#include "categoriesselector.h"
#include <QCoreApplication>
#include <QSettings>
#include "vgrabber.h"
#include "categorymodifier.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace vGrabber
		{
			CategoriesSelector::CategoriesSelector (Type type,
					vGrabber *vgr, QWidget *parent)
			: QWidget (parent)
			, Parent_ (vgr)
			, Type_ (type)
			{
				Ui_.setupUi (this);

				ReadSettings ();
			}

			QStringList CategoriesSelector::GetCategories () const
			{
				QStringList categories;
				for (int i = 0, size = Ui_.CategoriesTree_->topLevelItemCount ();
						i < size; ++i)
				{
					QTreeWidgetItem *item = Ui_.CategoriesTree_->topLevelItem (i);
					categories << item->data (0, Qt::UserRole).toString ();
				}
				return categories;
			}

			QStringList CategoriesSelector::GetHRCategories () const
			{
				QStringList result;
				Q_FOREACH (QString id, GetCategories ())
					result << Parent_->GetProxy ()->
						GetTagsManager ()->GetTag (id);
				return result;
			}

			void CategoriesSelector::ReadSettings ()
			{
				Ui_.CategoriesTree_->clear ();

				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_vGrabber");
				settings.beginGroup ("Categories");

				int size = settings.beginReadArray (QString::number (Type_));
				QList<QTreeWidgetItem*> items;
				for (int i = 0; i < size; ++i)
				{
					settings.setArrayIndex (i);
					QString id = settings.value ("ID").toString ();
					QString name = Parent_->GetProxy ()->
						GetTagsManager ()->GetTag (id);

					QTreeWidgetItem *item = new QTreeWidgetItem (Ui_.CategoriesTree_,
							QStringList (name));
					item->setData (0, Qt::UserRole, id);
					items << item;
				}

				if (items.size ())
					Ui_.CategoriesTree_->addTopLevelItems (items);
				else
				{
					switch (Type_)
					{
						case TAudio:
							AddItem ("music");
							WriteSettings ();
							Deleted_.clear ();
							Added_.clear ();
							break;
						case TVideo:
							AddItem ("videos");
							WriteSettings ();
							Deleted_.clear ();
							Added_.clear ();
							break;
					}
				}

				settings.endArray ();
				settings.endGroup ();
			}

			void CategoriesSelector::WriteSettings ()
			{
				qDebug () << Q_FUNC_INFO;
				QSettings settings (QCoreApplication::organizationName (),
						QCoreApplication::applicationName () + "_vGrabber");
				settings.beginGroup ("Categories");
				settings.beginWriteArray (QString::number (Type_));
				for (int i = 0, size = Ui_.CategoriesTree_->topLevelItemCount ();
						i < size; ++i)
				{
					settings.setArrayIndex (i);
					QTreeWidgetItem *item = Ui_.CategoriesTree_->topLevelItem (i);
					settings.setValue ("ID",
							item->data (0, Qt::UserRole).toString ());
				}
				settings.endArray ();
				settings.endGroup ();
			}
			
			void CategoriesSelector::AddItem (const QString& name)
			{
				QString id = Parent_->GetProxy ()->
					GetTagsManager ()->GetID (name);

				QTreeWidgetItem *item = new QTreeWidgetItem (Ui_.CategoriesTree_,
						QStringList (name));
				item->setData (0, Qt::UserRole, id);
				Ui_.CategoriesTree_->addTopLevelItem (item);

				if (Deleted_.contains (id))
					Deleted_.removeAll (id);
				else
					Added_ << id;
			}

			void CategoriesSelector::accept ()
			{
				qDebug () << Q_FUNC_INFO << this;

				WriteSettings ();

				emit goingToAccept (Added_, Deleted_);

				Deleted_.clear ();
				Added_.clear ();
			}

			void CategoriesSelector::reject ()
			{
				qDebug () << Q_FUNC_INFO << this;
				ReadSettings ();

				Deleted_.clear ();
				Added_.clear ();
			}

			void CategoriesSelector::on_Add__released ()
			{
				CategoryModifier cm (QString (), this);
				cm.setWindowTitle (tr ("Add category"));
				if (cm.exec () != QDialog::Accepted)
					return;

				QStringList splitted = Parent_->GetProxy ()->
					GetTagsManager ()->Split (cm.GetText ());
				Q_FOREACH (QString cat, splitted)
					AddItem (cat);
			}

			void CategoriesSelector::on_Modify__released ()
			{
				QTreeWidgetItem *item = Ui_.CategoriesTree_->currentItem ();
				if (!item)
					return;

				CategoryModifier cm (QString (item->text (0)));
				cm.setWindowTitle (tr ("Modify category"));
				if (cm.exec () != QDialog::Accepted)
					return;

				QStringList splitted = Parent_->GetProxy ()->
					GetTagsManager ()->Split (cm.GetText ());
				Q_FOREACH (QString cat, splitted)
					AddItem (cat);

				QString id = item->data (0, Qt::UserRole).toString ();
				if (Added_.contains (id))
					Added_.removeAll (id);
				else
					Deleted_ << id;
				delete item;
			}

			void CategoriesSelector::on_Remove__released ()
			{
				QTreeWidgetItem *item = Ui_.CategoriesTree_->currentItem ();
				if (item &&
						Ui_.CategoriesTree_->topLevelItemCount () > 1)
				{
					QString id = item->data (0, Qt::UserRole).toString ();
					if (Added_.contains (id))
						Added_.removeAll (id);
					else
						Deleted_ << id;
					delete item;
				}
			}
		};
	};
};

