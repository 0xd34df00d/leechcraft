/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "gstutil.h"
#include <QMap>
#include <QString>
#include <QtDebug>
#include <gst/gst.h>

namespace LC
{
namespace LMP
{
namespace GstUtil
{
	void AddGhostPad (GstElement *from, GstElement *to, const char *name)
	{
		auto pad = gst_element_get_static_pad (from, name);
		auto ghostPad = gst_ghost_pad_new (name, pad);
		gst_pad_set_active (ghostPad, TRUE);
		gst_element_add_pad (to, ghostPad);
		gst_object_unref (pad);
	}

	namespace
	{
		struct TagFunctionData
		{
			TagMap_t& Map_;
		};

		void TagFunction (const GstTagList *list, const gchar *tag, gpointer rawData)
		{
			const auto& data = static_cast<TagFunctionData*> (rawData);
			auto& map = data->Map_;

			const auto& tagName = QString::fromUtf8 (tag).toLower ();
			auto& valList = map [tagName];

			switch (gst_tag_get_type (tag))
			{
			case G_TYPE_STRING:
			{
				gchar *str = nullptr;
				gst_tag_list_get_string (list, tag, &str);
				valList = QString::fromUtf8 (str);
				g_free (str);
				break;
			}
			case G_TYPE_BOOLEAN:
			{
				int val = 0;
				gst_tag_list_get_boolean (list, tag, &val);
				valList = QString::number (val);
				break;
			}
			case G_TYPE_INT:
			{
				int val = 0;
				gst_tag_list_get_int (list, tag, &val);
				valList = QString::number (val);
				break;
			}
			case G_TYPE_UINT:
			{
				uint val = 0;
				gst_tag_list_get_uint (list, tag, &val);
				valList = QString::number (val);
				break;
			}
			case G_TYPE_FLOAT:
			{
				float val = 0;
				gst_tag_list_get_float (list, tag, &val);
				valList = QString::number (val);
				break;
			}
			case G_TYPE_DOUBLE:
			{
				double val = 0;
				gst_tag_list_get_double (list, tag, &val);
				valList = QString::number (val);
				break;
			}
			default:
				break;
			}
		}
	}

	bool ParseTagMessage (GstMessage *msg, TagMap_t& map, const QString& region)
	{
		GstTagList *tagList = nullptr;
		gst_message_parse_tag (msg, &tagList);
		if (!tagList)
			return false;

		TagFunctionData data { map };

		gst_tag_list_foreach (tagList,
				TagFunction,
				&data);
		gst_tag_list_unref (tagList);
		return true;
	}

	namespace
	{
		struct CallbackData
		{
			const std::function<void ()> Functor_;
		};

		GstPadProbeReturn ProbeHandler (GstPad*, GstPadProbeInfo*, gpointer cbDataPtr)
		{
			const auto cbData = static_cast<CallbackData*> (cbDataPtr);
			cbData->Functor_ ();
			delete cbData;
			return GST_PAD_PROBE_REMOVE;
		}
	}

	void PerformWProbe (GstPad *srcpad, const std::function<void ()>& functor)
	{
		gst_pad_add_probe (srcpad, GST_PAD_PROBE_TYPE_IDLE, ProbeHandler, new CallbackData { functor }, nullptr);
	}

	void DebugPrintState (GstElement *elem, GstClockTime time)
	{
		GstState state, pending;
		gst_element_get_state (elem, &state, &pending, time);
		qDebug () << state << pending;
	}

	const char* GetTeePadTemplateName ()
	{
		return "src_%u";
	}
}
}
}
