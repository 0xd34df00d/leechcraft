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

#include "localbloggingplatform.h"
#include <QIcon>
#include <QInputDialog>
#include <QSettings>
#include <QtDebug>
#include <QMainWindow>
#include <interfaces/core/irootwindowsmanager.h>
#include <util/passutils.h>
#include "core.h"

namespace LeechCraft
{
namespace Blogique
{
namespace Hestia
{
	LocalBloggingPlatform::LocalBloggingPlatform (QObject *parent)
	: QObject (parent)
	, ParentBlogginPlatfromPlugin_ (parent)
	, PluginProxy_ (0)
	{
	}

	QObject* LocalBloggingPlatform::GetObject ()
	{
		return this;
	}

	IBloggingPlatform::BloggingPlatfromFeatures LocalBloggingPlatform::GetFeatures () const
	{
		return BPFLocalBlog;
	}

	QObjectList LocalBloggingPlatform::GetRegisteredAccounts ()
	{
		QObjectList result;
		return result;
	}

	QObject* LocalBloggingPlatform::GetParentBloggingPlatformPlugin () const
	{
		return ParentBlogginPlatfromPlugin_;
	}

	QString LocalBloggingPlatform::GetBloggingPlatformName () const
	{
		return "Local blog";
	}

	QIcon LocalBloggingPlatform::GetBloggingPlatformIcon () const
	{
		return QIcon ();
	}

	QByteArray LocalBloggingPlatform::GetBloggingPlatformID () const
	{
		return "Blogique.Hestia.LocalBlog";
	}

	QList<QWidget*> LocalBloggingPlatform::GetAccountRegistrationWidgets (IBloggingPlatform::AccountAddOptions)
	{
		QList<QWidget*> result;
		return result;
	}

	void LocalBloggingPlatform::RegisterAccount (const QString& name,
			const QList<QWidget*>& widgets)
	{
	}

	void LocalBloggingPlatform::RemoveAccount (QObject *account)
	{
	}

	QList<QAction*> LocalBloggingPlatform::GetEditorActions () const
	{
		return QList<QAction*> ();
	}

	QList<QWidget*> LocalBloggingPlatform::GetBlogiqueSideWidgets () const
	{
		return QList<QWidget*> ();
	}

	void LocalBloggingPlatform::SetPluginProxy (QObject *proxy)
	{
		PluginProxy_ = proxy;
	}

	void LocalBloggingPlatform::Prepare ()
	{
		RestoreAccounts ();
	}

	void LocalBloggingPlatform::Release ()
	{
		saveAccounts ();
	}

	void LocalBloggingPlatform::RestoreAccounts ()
	{
	}

	void LocalBloggingPlatform::saveAccounts ()
	{
	}
}
}
}
