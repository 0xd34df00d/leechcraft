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

#include "passwordremember.h"
#include <QtDebug>
#include <plugininterface/util.h>
#include "core.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			PasswordRemember::PasswordRemember (QWidget *parent)
			: Notification (parent)
			{
				Ui_.setupUi (this);

				connect (this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)),
						&Core::Instance (),
						SIGNAL (delegateEntity (const LeechCraft::Entity&, int*, QObject**)));
			}

			void PasswordRemember::add (const PageFormsData_t& data)
			{
				// If there is already some data awaiting for user
				// response, don't add new one.
				if (TempData_.size ())
					return;

				TempData_ = data;

				show ();
			}

			void PasswordRemember::on_Remember__released ()
			{
				QList<QVariant> keys;
				QList<QVariant> values;
				Q_FOREACH (const QString& key, TempData_.keys ())
				{
					keys << "org.LeechCraft.Poshuku.Forms.InputByName/" + key.toUtf8 ();
					values << QVariant::fromValue<ElementsData_t> (TempData_ [key]);
				}
				if (keys.size ())
				{
					Entity e = Util::MakeEntity (keys,
							QString (),
							Internal,
							"x-leechcraft/data-persistent-save");
					e.Additional_ ["Values"] = values;
					emit delegateEntity (e, 0, 0);
				}

				TempData_.clear ();

				hide ();
			}

			void PasswordRemember::on_NotNow__released ()
			{
				TempData_.clear ();
				hide ();
			}

			void PasswordRemember::on_Never__released ()
			{
				if (TempData_.size ())
				{
					QSet<QString> urls;
					Q_FOREACH (const QString& key, TempData_.keys ())
						Q_FOREACH (const ElementData& ed, TempData_ [key])
							urls << ed.PageURL_.toString ();

					Q_FOREACH (const QString& url, urls)
						Core::Instance ().GetStorageBackend ()->
							SetFormsIgnored (url, true);
				}

				TempData_.clear ();
				hide ();
			}
		};
	};
};

