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

#include "protomanager.h"
#include <util/util.h>
#include <libpurple/purple.h>
#include <libpurple/core.h>
#include <libpurple/plugin.h>
#include "protocol.h"
#include "account.h"

namespace LeechCraft
{
namespace Azoth
{
namespace VelvetBird
{
	namespace
	{
		GHashTable* GetUIInfo ()
		{
			static GHashTable *uiInfo = 0;
			if (!uiInfo)
			{
				uiInfo = g_hash_table_new (g_str_hash, g_str_equal);
				auto add = [uiInfo] (char *name, char *value)
				{
					g_hash_table_insert (uiInfo, name, value);
				};
				add ("name", "LeechCraft VelvetBird");
				add ("version", "dummy");
				add ("website", "http://leechcraft.org");
				add ("dev_website", "http://leechcraft.org");
				add ("client_type", "pc");
			}
			return uiInfo;
		}

		PurpleCoreUiOps UiOps =
		{
			NULL,
			NULL,
			NULL,
			NULL,
			GetUIInfo,

			NULL,
			NULL,
			NULL
		};

		const auto PurpleReadCond = G_IO_IN | G_IO_HUP | G_IO_ERR;
		const auto PurpleWriteCond = G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL;

		struct InputClosure
		{
			PurpleInputFunction F_;
			guint Result_;
			gpointer Data_;
		};

		guint glib_input_add (gint fd, PurpleInputCondition condition,
				PurpleInputFunction function, gpointer data)
		{
			int cond = 0;

			if (condition & PURPLE_INPUT_READ)
				cond |= PurpleReadCond;
			if (condition & PURPLE_INPUT_WRITE)
				cond |= PurpleWriteCond;

			auto closure = new InputClosure { function, 0, data };

			auto channel = g_io_channel_unix_new (fd);
			auto res = g_io_add_watch_full (channel,
					G_PRIORITY_DEFAULT,
					static_cast<GIOCondition> (cond),
					[] (GIOChannel *source, GIOCondition condition, gpointer data) -> gboolean
					{
						int cond = 0;
						if (condition & PURPLE_INPUT_READ)
							cond |= PurpleReadCond;
						if (condition & PURPLE_INPUT_WRITE)
							cond |= PurpleWriteCond;

						auto closure = static_cast<InputClosure*> (data);
						closure->F_ (closure->Data_,
								g_io_channel_unix_get_fd (source),
								static_cast<PurpleInputCondition> (cond));
						return true;
					},
					closure,
					[] (gpointer data) { delete static_cast<InputClosure*> (data); });

			g_io_channel_unref(channel);
			return res;
		}

		PurpleEventLoopUiOps EvLoopOps =
		{
			g_timeout_add,
			g_source_remove,
			glib_input_add,
			g_source_remove,
			NULL,
			g_timeout_add_seconds,

			NULL,
			NULL,
			NULL
		};

		PurpleIdleUiOps IdleOps =
		{
			[] () { return time_t (); }
		};

		PurpleAccountUiOps AccUiOps =
		{
			NULL,
			[] (PurpleAccount *acc, PurpleStatus *status)
				{ static_cast<Account*> (acc->ui_data)->HandleStatus (status); },
			NULL,
			NULL,
			NULL,

			NULL,
			NULL,
			NULL,
			NULL
		};
	}

	ProtoManager::ProtoManager (ICoreProxy_ptr proxy, QObject *parent)
	: QObject (parent)
	, Proxy_ (proxy)
	{
		const auto& dir = Util::CreateIfNotExists ("azoth/velvetbird/purple");
		purple_util_set_user_dir (dir.absolutePath ().toUtf8 ().constData ());

		purple_core_set_ui_ops (&UiOps);
		purple_eventloop_set_ui_ops (&EvLoopOps);
		purple_idle_set_ui_ops (&IdleOps);

		if (!purple_core_init ("leechcraft.azoth"))
		{
			qWarning () << Q_FUNC_INFO
					<< "failed initializing libpurple";
			return;
		}

		purple_accounts_set_ui_ops (&AccUiOps);

		QMap<QByteArray, Protocol*> id2proto;

		auto protos = purple_plugins_get_protocols ();
		while (protos)
		{
			auto item = static_cast<PurplePlugin*> (protos->data);
			protos = protos->next;

			Protocols_ << new Protocol (item, proxy, parent);
			id2proto [Protocols_.last ()->GetPurpleID ()] = Protocols_.last ();
		}

		auto accs = purple_accounts_get_all ();
		while (accs)
		{
			auto acc = static_cast<PurpleAccount*> (accs->data);
			accs = accs->next;

			id2proto [purple_account_get_protocol_id (acc)]->PushAccount (acc);
		}
	}

	QList<QObject*> ProtoManager::GetProtoObjs () const
	{
		QList<QObject*> result;
		for (auto proto : Protocols_)
			result << proto;
		return result;
	}
}
}
}
