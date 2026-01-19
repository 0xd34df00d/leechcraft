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
#include <interfaces/azoth/itransfermanager.h>
#include "types.h"

namespace LC::Azoth::Sarin
{
	class ToxAccount;
	class ToxRunner;

	class FileTransferManager : public QObject
							  , public ITransferManager
	{
		Q_OBJECT
		Q_INTERFACES (LC::Azoth::ITransferManager)

		ToxAccount * const Acc_;
		std::weak_ptr<ToxRunner> Tox_;
	public:
		explicit FileTransferManager (ToxAccount*);

		bool IsAvailable () const override;
		QObject* SendFile (const QString&, const QString&, const QString&, const QString&) override;

		void HandleToxThreadChanged (const std::shared_ptr<ToxRunner>&);
	private:
		void HandleRequest (uint32_t, Pubkey, uint32_t, uint64_t, const QString&);
	signals:
		void fileOffered (QObject*) override;

		// TODO handle these signals in this Manager instead of re-emitting them
		void gotFileControl (uint32_t, uint32_t, int);
		void gotData (quint32, quint32, const QByteArray&, uint64_t);
		void gotChunkRequest (uint32_t friendNum, uint32_t fileNum, uint64_t position, size_t length);
	};
}
