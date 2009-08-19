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

#include "jsproxy.h"
#include <QString>
#include <QVariant>
#include <QTextCodec>
#include <QtDebug>
#include <plugininterface/util.h>

namespace LeechCraft
{
	namespace Plugins
	{
		namespace Poshuku
		{
			JSProxy::JSProxy (QObject *parent)
			: QObject (parent)
			{
			}
			
			PageFormsData_t JSProxy::GetForms () const
			{
				return Current_;
			}
			
			void JSProxy::SetForms (const PageFormsData_t& data)
			{
				Current_ = data;
			}
			
			void JSProxy::ClearForms ()
			{
				Current_.clear ();
			}

			void JSProxy::debug (const QString& string)
			{
				qDebug () << Q_FUNC_INFO << string;
			}
			
			void JSProxy::warning (const QString& string)
			{
				qWarning () << Q_FUNC_INFO << string;
			}
			
			void JSProxy::setFormElement (const QString& url,
					int formId,
					const QString& elemName,
					const QString& elemType,
					const QVariant& value)
			{
				ElementData ed =
				{
					formId,
					elemName,
					elemType,
					value
				};
				Current_ [url] << ed;
			}
			
			struct FormMatcher
			{
				int FormID_;
			
				FormMatcher (int id)
				: FormID_ (id)
				{
				}
			
				bool operator() (const ElementData& ed) const
				{
					return ed.FormIndex_ == FormID_;
				}
			};
			
			QVariant JSProxy::getFormElement (int formId,
					const QString& elemName,
					const QString& elemType) const
			{
				ElementsData_t elems = Current_.values ().at (0);
				ElementsData_t filtered;
			
				LeechCraft::Util::copy_if (elems.begin (), elems.end (),
						std::back_inserter (filtered),
						ElemFinder (elemName, elemType));
			
				if (filtered.size () == 1)
					return QTextCodec::codecForName ("UTF-8")->
						toUnicode (filtered.at (0).Value_.toByteArray ());
				else
				{
					ElementsData_t::const_iterator pos =
						std::find_if (filtered.begin (), filtered.end (),
								FormMatcher (formId));
					if (pos == filtered.end ())
						return QVariant ();
					else
						return QTextCodec::codecForName ("UTF-8")->
							toUnicode (pos->Value_.toByteArray ());
				}
			}
		};
	};
};

