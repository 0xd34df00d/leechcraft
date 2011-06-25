/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
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

#include "confwidget.h"
#include "xmlsettingsmanager.h"

namespace LeechCraft
{
namespace Azoth
{
namespace Herbicide
{
	ConfWidget::ConfWidget (QWidget *parent)
	: QWidget (parent)
	{
		Ui_.setupUi (this);
		
		LoadSettings ();
	}
	
	QString ConfWidget::GetQuestion () const
	{
		return Ui_.Question_->toPlainText ();
	}
	
	QStringList ConfWidget::GetAnswers () const
	{
		return Ui_.Answers_->toPlainText ()
			.split ('\n', QString::SkipEmptyParts);
	}
	
	void ConfWidget::SaveSettings () const
	{
		XmlSettingsManager::Instance ().setProperty ("Question", GetQuestion ());
		XmlSettingsManager::Instance ().setProperty ("Answers", GetAnswers ());
	}

	void ConfWidget::LoadSettings ()
	{
		const QString& question = XmlSettingsManager::Instance ()
				.property ("Question").toString ();
		Ui_.Question_->setPlainText (question);

		const QStringList& answers = XmlSettingsManager::Instance ()
				.property ("Answers").toStringList ();
		Ui_.Answers_->setPlainText (answers.join ("\n"));
	}
	
	void ConfWidget::accept ()
	{
		SaveSettings ();
	}
	
	void ConfWidget::reject ()
	{
		LoadSettings ();
	}
}
}
}
