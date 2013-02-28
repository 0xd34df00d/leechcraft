/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2012  Oleg Linkin
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

#include "postoptionswidget.h"
#include <QtDebug>
#include <util/util.h>
#include "localblogaccount.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Hestia
{
	PostOptionsWidget::PostOptionsWidget (QWidget *parent)
	: QWidget (parent)
	, Account_ (0)
	{
		Ui_.setupUi (this);
	}

	QString PostOptionsWidget::GetName () const
	{
		return tr ("Post options");
	}

	SideWidgetType PostOptionsWidget::GetWidgetType () const
	{
		return SideWidgetType::PostOptionsSideWidget;
	}

	QVariantMap PostOptionsWidget::GetPostOptions () const
	{
		return QVariantMap ();
	}

	void PostOptionsWidget::SetPostOptions (const QVariantMap& map)
	{
	}

	QVariantMap PostOptionsWidget::GetCustomData () const
	{
		return QVariantMap ();
	}

	void PostOptionsWidget::SetCustomData (const QVariantMap&)
	{
	}

	void PostOptionsWidget::SetAccount (QObject *accObj)
	{
		Account_ = qobject_cast<LocalBlogAccount*> (accObj);
		if (!Account_)
		{
			qWarning () << Q_FUNC_INFO
					<< "account"
					<< accObj
					<< "doesn't belong to LocalBlog";
			return;
		}
	}

	QStringList PostOptionsWidget::GetTags () const
	{
		QStringList tags;
		for (auto tag : Ui_.Tags_->text ().split (","))
			tags << tag.trimmed ();
		return tags;
	}

	void PostOptionsWidget::SetTags (const QStringList& tags)
	{
		Ui_.Tags_->setText (tags.join (", "));
	}

	QDateTime PostOptionsWidget::GetPostDate () const
	{
		return !Ui_.TimestampBox_->isChecked () ?
				QDateTime::currentDateTime () :
				QDateTime (QDate (Ui_.Year_->value (),
							Ui_.Month_->currentIndex () + 1,
							Ui_.Date_->value ()),
						Ui_.Time_->time ());
	}

	void PostOptionsWidget::SetPostDate (const QDateTime& date)
	{
		Ui_.TimestampBox_->setChecked (true);
		Ui_.Year_->setValue (date.date ().year ());
		Ui_.Month_->setCurrentIndex (date.date ().month () - 1);
		Ui_.Date_->setValue (date.date ().day ());
		Ui_.Time_->setTime (date.time ());
	}

	void PostOptionsWidget::on_CurrentTime__released ()
	{
		QDateTime current = QDateTime::currentDateTime ();
		Ui_.Year_->setValue (current.date ().year ());
		Ui_.Month_->setCurrentIndex (current.date ().month () - 1);
		Ui_.Date_->setValue (current.date ().day ());
		Ui_.Time_->setTime (current.time ());
	}
}
}
}
