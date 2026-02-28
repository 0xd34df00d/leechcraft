/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <memory>
#include <QObject>
#include <projectM-4/projectM.h>
#include <projectM-4/playlist.h>
#include <interfaces/lmp/ifilterelement.h>
#include <interfaces/lmp/ilmpproxy.h>

typedef struct _GstPad GstPad;
typedef struct _GstBuffer GstBuffer;

namespace LC
{
namespace LMP
{
namespace Potorchu
{
	class VisWidget;
	class VisScene;

	class VisualFilter : public QObject
					   , public IFilterElement
	{
		Q_OBJECT

		const QByteArray EffectId_;
		const ILMPProxy_ptr LmpProxy_;

		const std::shared_ptr<VisWidget> Widget_;
		const std::shared_ptr<VisScene> Scene_;

		GstElement * const Elem_;
		GstElement * const Tee_;
		GstElement * const AudioQueue_;
		GstElement * const ProbeQueue_;
		GstElement * const Converter_;
		GstElement * const FakeSink_;
		projectm_handle ProjectM_ = nullptr;
		projectm_playlist_handle Playlist_ = nullptr;
	public:
		VisualFilter (const QByteArray&, const ILMPProxy_ptr&);
		~VisualFilter ();

		QByteArray GetEffectId () const;
		QByteArray GetInstanceId () const;
		IFilterConfigurator* GetConfigurator () const;
	protected:
		GstElement* GetElement () const;
	private:
		void InitProjectM ();

		void HandleBuffer (GstBuffer*);
	private slots:
		void handleSceneRectChanged (const QRectF&);

		void updateFrame ();

		void handlePrevVis ();
		void handleNextVis ();
	};
}
}
}
