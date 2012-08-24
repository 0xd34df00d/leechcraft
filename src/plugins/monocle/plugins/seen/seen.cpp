/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
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

#include "seen.h"
#include <QIcon>
#include "document.h"

namespace LeechCraft
{
namespace Monocle
{
namespace Seen
{
	namespace
	{
		void MsgCallback (ddjvu_context_t*, void *closure)
		{
			auto plugin = static_cast<Plugin*> (closure);
			QMetaObject::invokeMethod (plugin,
					"checkMessageQueue",
					Qt::QueuedConnection);
		}
	}

	void Plugin::Init (ICoreProxy_ptr proxy)
	{
		Context_ = ddjvu_context_create ("leechcraft");
		ddjvu_message_set_callback (Context_, MsgCallback, this);
	}

	void Plugin::SecondInit ()
	{
	}

	QByteArray Plugin::GetUniqueID () const
	{
		return "org.LeechCraft.Monocle.Seen";
	}

	void Plugin::Release ()
	{
		ddjvu_context_release (Context_);
	}

	QString Plugin::GetName () const
	{
		return "Monocle Seen";
	}

	QString Plugin::GetInfo () const
	{
		return tr ("DjVu backend for Monocle.");
	}

	QIcon Plugin::GetIcon () const
	{
		return QIcon ();
	}

	QSet<QByteArray> Plugin::GetPluginClasses () const
	{
		QSet<QByteArray> result;
		result << "org.LeechCraft.Monocle.IBackendPlugin";
		return result;
	}

	bool Plugin::CanLoadDocument (const QString& file)
	{
		return file.toLower ().endsWith (".djvu");
	}

	IDocument_ptr Plugin::LoadDocument (const QString& file)
	{
		qDebug () << Q_FUNC_INFO << "requested opening" << file;
		IDocument_ptr doc (new Document (file, Context_));
		ddjvu_message_wait (Context_);
		return doc;
	}

	void Plugin::checkMessageQueue ()
	{
		while (const ddjvu_message_t *msg = ddjvu_message_peek (Context_))
		{
			qDebug () << Q_FUNC_INFO << msg->m_any.tag;

			ddjvu_message_pop (Context_);
		}
	}
}
}
}

LC_EXPORT_PLUGIN (leechcraft_monocle_seen, LeechCraft::Monocle::Seen::Plugin);
