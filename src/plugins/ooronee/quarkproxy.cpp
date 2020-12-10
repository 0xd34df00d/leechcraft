/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "quarkproxy.h"
#include <QInputDialog>
#include <QMenu>
#include <QUrl>
#include <util/sll/prelude.h>
#include <util/sll/qtutil.h>
#include <util/xpc/util.h>
#include <util/xpc/stddatafiltermenucreator.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ientitymanager.h>
#include <interfaces/idatafilter.h>
#include <interfaces/ientityhandler.h>
#include "xmlsettingsmanager.h"

namespace LC
{
namespace Ooronee
{
	QuarkProxy::QuarkProxy (ICoreProxy_ptr proxy)
	: Proxy_ { proxy }
	{
		XmlSettingsManager::Instance ().RegisterObject ("HoverTimeout",
				this, "hoverTimeoutChanged");
	}

	int QuarkProxy::GetHoverTimeout () const
	{
		return XmlSettingsManager::Instance ().property ("HoverTimeout").toInt ();
	}

	void QuarkProxy::Handle (const QVariant& data, const QByteArray& typeId, bool menuSelect)
	{
		if (menuSelect)
		{
			HandleVariantsMenu (data, typeId);
			return;
		}

		const auto getId = [&typeId] (const QByteArray& prefix)
		{
			return XmlSettingsManager::Instance ().Property (Util::AsStringView (prefix + typeId), {}).toByteArray ();
		};
		const auto& prevPluginId = getId ("PrevHandler");
		const auto& prevVariantId = getId ("PrevVariant");

		auto entity = Util::MakeEntity (data,
				{},
				TaskParameter::NoParameters,
				"x-leechcraft/data-filter-request");

		QList<VarInfo> varInfos;

		const auto& objs = Proxy_->GetEntityManager ()->GetPossibleHandlers (entity);
		for (const auto& obj : objs)
		{
			const auto& pluginId = qobject_cast<IInfo*> (obj)->GetUniqueID ();
			const auto idf = qobject_cast<IDataFilter*> (obj);
			if (!idf)
				continue;

			for (const auto& var : idf->GetFilterVariants (data))
			{
				if (pluginId == prevPluginId &&
						var.ID_ == prevVariantId)
				{
					entity.Additional_ ["DataFilter"] = var.ID_;
					qobject_cast<IEntityHandler*> (obj)->Handle (entity);

					return;
				}

				varInfos.append ({
						idf->GetFilterVerb () + ": " + var.Name_,
						obj,
						var.ID_
					});
			}
		}

		const auto& strings = Util::Map (varInfos, &VarInfo::HumanReadable_);
		HandleVariantsDialog (entity, strings, varInfos, typeId);
	}

	void QuarkProxy::HandleVariantsMenu (const QVariant& data, const QByteArray& typeId)
	{
		QMenu menu;
		Util::StdDataFilterMenuCreator creator { data, Proxy_->GetEntityManager (), &menu };
		menu.exec (QCursor::pos ());

		if (menu.actions ().isEmpty ())
			return;

		SaveUsed (creator.GetChosenPlugin (), creator.GetChosenVariant (), typeId);
	}

	namespace
	{
		QString GetTypeString (const QVariant& data)
		{
			switch (data.type ())
			{
			case QVariant::Image:
				return QuarkProxy::tr ("Select the data filter to handle the dropped image:");
			case QVariant::String:
				return QuarkProxy::tr ("Select the data filter to handle the dropped text:");
			case QVariant::Url:
				return data.toUrl ().isLocalFile () ?
						QuarkProxy::tr ("Select the data filter to handle the dropped file:") :
						QuarkProxy::tr ("Select the data filter to handle the dropped URL:");
			default:
				return QuarkProxy::tr ("Select the data filter to handle the dropped data:");
			}
		}
	}

	void QuarkProxy::HandleVariantsDialog (Entity entity,
			const QStringList& strings, const QList<VarInfo>& varInfos, const QByteArray& typeId)
	{
		if (varInfos.isEmpty ())
			return;

		bool ok = false;
		const auto& selected = QInputDialog::getItem (nullptr,
				tr ("Handle dropped data"),
				GetTypeString (entity.Entity_),
				strings,
				0,
				false,
				&ok);
		if (!ok)
			return;

		const auto idx = strings.indexOf (selected);
		if (idx < 0)
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot find"
					<< idx;
			return;
		}

		const auto& varInfo = varInfos.at (idx);

		entity.Additional_ ["DataFilter"] = varInfo.Variant_;
		qobject_cast<IEntityHandler*> (varInfo.Obj_)->Handle (entity);

		SaveUsed (qobject_cast<IInfo*> (varInfo.Obj_)->GetUniqueID (), varInfo.Variant_, typeId);
	}

	void QuarkProxy::SaveUsed (const QByteArray& plugin,
			const QByteArray& variant, const QByteArray& typeId)
	{
		XmlSettingsManager::Instance ().setProperty ("PrevHandler" + typeId, plugin);
		XmlSettingsManager::Instance ().setProperty ("PrevVariant" + typeId, variant);
	}

	namespace
	{
		QByteArray GetTypeId (const QVariant& data)
		{
			switch (data.type ())
			{
			case QVariant::Image:
				return "Image";
			case QVariant::ByteArray:
				return "ByteArray";
			case QVariant::String:
				return "String";
			default:
				return "Other";
			}
		}
	}

	void QuarkProxy::handle (const QVariant& data, bool menuSelect)
	{
		Handle (data, GetTypeId (data), menuSelect);
	}
}
}
