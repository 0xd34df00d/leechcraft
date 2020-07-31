/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "sourceobject.h"
#include "interfaces/lmp/ipath.h"

namespace LC
{
namespace LMP
{
	class Output;

	typedef std::function<int (GstBus*, GstMessage*)> SyncHandler_f;

	class Path : public QObject
			   , public IPath
	{
		SourceObject * const SrcObj_;

		GstElement * const WholeBin_;
		GstElement * const Identity_;

		GstElement *Pipeline_ = nullptr;
		GstElement *OutputBin_ = nullptr;

		QList<GstElement*> NextWholeElems_;

		enum class Action
		{
			Add,
			Remove
		};

		struct QueueItem
		{
			GstElement *Elem_;
			Action Act_;
		};
		QList<QueueItem> Queue_;
	public:
		Path (SourceObject*, Output*, QObject* = 0);

		GstElement* GetPipeline () const;
		void SetPipeline (GstElement*);

		GstElement* GetOutPlaceholder () const;
		GstElement* GetWholeOut () const;

		GstElement* GetOutputBin () const;
		void SetOutputBin (GstElement*);

		SourceObject* GetSourceObject () const;

		void AddSyncHandler (const SyncHandler_f&, QObject*);
		void AddAsyncHandler (const AsyncHandler_f&, QObject*);

		void InsertElement (GstElement*);
		void RemoveElement (GstElement*);

		void FinalizeAction ();
	private:
		void RotateQueue ();
	};
}
}
