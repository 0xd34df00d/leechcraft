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

#include "livejournalbloggingplatform.h"
#include <QIcon>

namespace LeechCraft
{
namespace Blogique
{
namespace Metida
{
	LiveJournalBloggingPlatform::LiveJournalBloggingPlatform (QObject *parent)
	: QObject (parent)
	, ParentBlogginPlatfromPlugin_ (parent)
	{
	}

	QObject* LiveJournalBloggingPlatform::GetObject ()
	{
		return this;
	}

	QObjectList LiveJournalBloggingPlatform::GetRegisteredAccounts ()
	{
		return QObjectList ();
	}

	QObject* LiveJournalBloggingPlatform::GetParentBloggingPlatformPlugin () const
	{
		return ParentBlogginPlatfromPlugin_;
	}

	QString LiveJournalBloggingPlatform::GetBloggingPlatformName () const
	{
		return "LiveJournal";
	}

	QIcon LiveJournalBloggingPlatform::GetBloggingPlatformIcon () const
	{
		return QIcon (":/plugins/blogique/plugins/metida/resources/images/livejournalicon.svg");
	}

	QByteArray LiveJournalBloggingPlatform::GetBloggingPlatformID () const
	{
		return "Blogique.Metida.LiveJournal";
	}

	QList<QWidget*> LiveJournalBloggingPlatform::GetAccountRegistrationWidgets ()
	{
		return QList<QWidget*> ();
	}

	void LiveJournalBloggingPlatform::RegisterAccount (const QString& name,
			const QList<QWidget*>& widgets)
	{
	}

	void LiveJournalBloggingPlatform::RemoveAccount (QObject *account)
	{
	}
}
}
}