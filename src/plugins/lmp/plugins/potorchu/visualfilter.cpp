/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#include "visualfilter.h"
#include <QtDebug>
#include <QTemporaryFile>
#include <QWidget>
#include <QAction>
#include <gst/gst.h>
#include <libprojectM/projectM.hpp>
#include <util/sll/util.h>
#include <util/lmp/gstutil.h>
#include <interfaces/lmp/ilmpguiproxy.h>
#include "viswidget.h"
#include "visscene.h"

namespace LC
{
namespace LMP
{
namespace Potorchu
{
	VisualFilter::VisualFilter (const QByteArray& effectId, const ILMPProxy_ptr& proxy)
	: EffectId_ { effectId }
	, LmpProxy_ { proxy }
	, Widget_ { new VisWidget }
	, Scene_ { new VisScene }
	, Elem_ { gst_bin_new (nullptr) }
	, Tee_ { gst_element_factory_make ("tee", nullptr) }
	, AudioQueue_ { gst_element_factory_make ("queue", nullptr) }
	, ProbeQueue_ { gst_element_factory_make ("queue", nullptr) }
	, Converter_ { gst_element_factory_make ("audioconvert", nullptr) }
	, FakeSink_ { gst_element_factory_make ("fakesink", nullptr) }
	{
		gst_bin_add_many (GST_BIN (Elem_),
				Tee_, AudioQueue_,
				ProbeQueue_, Converter_, FakeSink_,
				nullptr);

		auto teeTemplate = gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (Tee_),
				GstUtil::GetTeePadTemplateName ());

		auto teeAudioPad = gst_element_request_pad (Tee_, teeTemplate, nullptr, nullptr);
		auto audioPad = gst_element_get_static_pad (AudioQueue_, "sink");
		gst_pad_link (teeAudioPad, audioPad);
		gst_object_unref (audioPad);

		GstUtil::AddGhostPad (Tee_, Elem_, "sink");
		GstUtil::AddGhostPad (AudioQueue_, Elem_, "src");

		gst_element_link (ProbeQueue_, Converter_);
		const auto caps = gst_caps_new_simple ("audio/x-raw",
				"format", G_TYPE_STRING, "S16LE",
				nullptr);
		gst_element_link_filtered (Converter_, FakeSink_, caps);
		gst_caps_unref (caps);

		auto teeProbePad = gst_element_request_pad (Tee_, teeTemplate, nullptr, nullptr);
		auto streamPad = gst_element_get_static_pad (ProbeQueue_, "sink");
		gst_pad_link (teeProbePad, streamPad);
		gst_object_unref (streamPad);

		Widget_->resize (512, 512);
		Widget_->setScene (Scene_.get ());
		Widget_->SetFps (30);

		connect (Widget_.get (),
				SIGNAL (redrawRequested ()),
				Scene_.get (),
				SLOT (update ()));
		connect (Scene_.get (),
				SIGNAL (redrawing ()),
				this,
				SLOT (updateFrame ()));
		connect (Scene_.get (),
				SIGNAL (sceneRectChanged (QRectF)),
				this,
				SLOT (handleSceneRectChanged (QRectF)));

		auto action = new QAction { tr ("Visualization"), this };
		action->setProperty ("ActionIcon", "view-media-visualization");
		action->setCheckable (true);
		connect (action,
				SIGNAL (triggered (bool)),
				Widget_.get (),
				SLOT (setVisible (bool)));
		proxy->GetGuiProxy ()->AddToolbarAction (action);

		connect (Widget_.get (),
				SIGNAL (nextVis ()),
				this,
				SLOT (handleNextVis ()));
		connect (Widget_.get (),
				SIGNAL (prevVis ()),
				this,
				SLOT (handlePrevVis ()));

		const auto srcpad = gst_element_get_static_pad (Converter_, "src");
		gst_pad_add_probe (srcpad,
				GST_PAD_PROBE_TYPE_BUFFER,
				[] (GstPad*, GstPadProbeInfo *info, gpointer filterPtr) -> GstPadProbeReturn
				{
					const auto filter = static_cast<VisualFilter*> (filterPtr);
					filter->HandleBuffer (GST_PAD_PROBE_INFO_BUFFER (info));
					return GST_PAD_PROBE_PASS;
				},
				this,
				nullptr);
	}

	QByteArray VisualFilter::GetEffectId () const
	{
		return EffectId_;
	}

	QByteArray VisualFilter::GetInstanceId () const
	{
		return EffectId_;
	}

	IFilterConfigurator* VisualFilter::GetConfigurator () const
	{
		return nullptr;
	}

	GstElement* VisualFilter::GetElement () const
	{
		return Elem_;
	}

	namespace
	{
		projectM::Settings makeSettings (const QRect& sceneRect)
		{
			std::unique_ptr<QTemporaryFile> fontFile
			{
				QTemporaryFile::createNativeFile (":/lmp/potorchu/resources/data/blank.ttf")
			};
			const std::string fontFileNameStr { fontFile->fileName ().toUtf8 ().constData () };

			projectM::Settings settings {};
			settings.meshX = 32;
			settings.meshY = 24;
			settings.fps = 30;
			settings.textureSize = 512;
			settings.windowWidth = sceneRect.width ();
			settings.windowHeight = sceneRect.height ();
			settings.presetURL = "/usr/share/projectM/presets";
			settings.titleFontURL = fontFileNameStr;
			settings.menuFontURL = fontFileNameStr;
			settings.smoothPresetDuration = 5;
			settings.presetDuration = 15;
			settings.beatSensitivity = 10;
			settings.aspectCorrection = false;
			settings.easterEgg = 0;
			settings.shuffleEnabled = true;
			settings.softCutRatingsEnabled = false;
			return settings;
		}
	}

	void VisualFilter::InitProjectM ()
	{
		auto settings = makeSettings (Scene_->sceneRect ().toRect ());
		ProjectM_.reset (new projectM { settings });
	}

	void VisualFilter::HandleBuffer (GstBuffer *buffer)
	{
		GstMapInfo map;
		if (!gst_buffer_map (buffer, &map, GST_MAP_READ))
		{
			qWarning () << Q_FUNC_INFO
					<< "cannot map data";
			return;
		}

		auto mapGuard = Util::MakeScopeGuard ([buffer, &map] { gst_buffer_unmap (buffer, &map); });

		const auto data = reinterpret_cast<const short*> (map.data);
		const auto size = map.size;

		const auto samples = size / sizeof (short) / 2;

		if (ProjectM_)
			ProjectM_->pcm ()->addPCM16Data (data, samples);
	}

	void VisualFilter::handleSceneRectChanged (const QRectF& rect)
	{
		if (ProjectM_)
			ProjectM_->projectM_resetGL (rect.width (), rect.height ());
	}

	void VisualFilter::updateFrame ()
	{
		if (!ProjectM_)
			InitProjectM ();

		ProjectM_->renderFrame ();
	}

	void VisualFilter::handleNextVis ()
	{
		ProjectM_->selectNext (true);
	}

	void VisualFilter::handlePrevVis ()
	{
		ProjectM_->selectPrevious (true);
	}
}
}
}
