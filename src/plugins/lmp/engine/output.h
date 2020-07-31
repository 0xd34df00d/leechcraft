/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QObject>
#include "pathelement.h"

typedef struct _GstElement GstElement;
typedef struct _GstPad GstPad;

namespace LC
{
namespace LMP
{
	class Path;

	class Output : public QObject
				 , public PathElement
	{
		Q_OBJECT

		GstElement *Bin_;
		GstElement *Volume_;

		bool SaveVolumeScheduled_ = false;
	public:
		Output (QObject* = 0);

		double GetVolume () const;
		bool IsMuted () const;
	protected:
		void AddToPath (Path*);
		void PostAdd (Path*);
	private:
		void ScheduleSaveVolume ();
	public slots:
		void setVolume (double);
		void setVolume (int);

		void toggleMuted ();
	private slots:
		void saveVolume ();
	signals:
		void volumeChanged (int);

		void mutedChanged (bool);
	};
}
}
