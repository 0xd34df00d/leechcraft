/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <vector>
#include <QObject>
#include "interfaces/lmp/ifilterelement.h"
#include "interfaces/lmp/isourceobject.h"

typedef struct _GstPad GstPad;
typedef struct _GstMessage GstMessage;
typedef struct _GstPadTemplate GstPadTemplate;

namespace LC
{
namespace LMP
{
namespace HttStream
{
	class HttpServer;
	class FilterConfigurator;

	class HttpStreamFilter : public QObject
						   , public IFilterElement
	{
		Q_OBJECT

		const QByteArray FilterId_;
		const QByteArray InstanceId_;
		IPath * const Path_;

		FilterConfigurator * const Configurator_;

		GstElement * const Elem_;

		GstElement * const Tee_;

		GstPadTemplate * const TeeTemplate_;

		GstElement * const AudioQueue_;
		GstElement * const StreamQueue_;
		GstElement * const AConv_;

		GstElement * const Encoder_;

		GstElement * const Muxer_;

		GstElement * const MSS_;

		HttpServer * const Server_;

		GstPad *TeeAudioPad_;
		GstPad *TeeStreamPad_ = nullptr;

		int ClientsCount_ = 0;

		SourceState StateOnFirst_ = SourceState::Error;
		QList<int> PendingSockets_;
	public:
		HttpStreamFilter (const QByteArray& filterId,
				const QByteArray& instanceId, IPath *path);
		~HttpStreamFilter ();

		QByteArray GetEffectId () const override;
		QByteArray GetInstanceId () const override;
		IFilterConfigurator* GetConfigurator () const override;

		void SetQuality (double);
		void SetAddress (const QString&, int);

		void HandleRemoved (int, int);
	protected:
		GstElement* GetElement () const override;
		void PostAdd (IPath*) override;
	private:
		void CreatePad ();
		void DestroyPad ();

		std::vector<GstElement*> GetStreamBranchElements () const;

		bool HandleFirstClientConnected ();
		void HandleLastClientDisconnected ();

		int HandleError (GstMessage*);
	private slots:
		void checkCreatePad (SourceState);

		void handleClient (int);
		void handleClientDisconnected (int);
	};
}
}
}
