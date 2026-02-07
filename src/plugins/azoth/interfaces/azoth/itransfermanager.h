/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <variant>
#include <QObject>
#include <QString>

namespace LC::Azoth::Emitters
{
	class TransferJob;
	class TransferManager;
}

namespace LC::Azoth
{
	namespace Transfers
	{
		enum class Phase : std::uint8_t
		{
			Starting,
			Transferring,
			Finished
		};

		enum class ErrorReason : std::uint8_t
		{
			Aborted,
			FileInaccessible,
			FileCorrupted,
			ProtocolError,
		};

		struct Error
		{
			ErrorReason Reason_;
			QString Message_ {};

			bool operator== (const Error&) const = default;
		};
	}

	using TransferState = std::variant<
			Transfers::Phase,
			Transfers::Error
		>;

	inline bool IsTerminal (const TransferState& state)
	{
		return std::holds_alternative<Transfers::Error> (state) || state == TransferState { Transfers::Phase::Finished };
	}

	/** @brief This interface must be implemented by objects
	 * representing file transfer jobs.
	 */
	class ITransferJob
	{
	public:
		virtual ~ITransferJob () = default;

		virtual Emitters::TransferJob& GetTransferJobEmitter () = 0;

		/** @brief Aborts a transfer.
		 *
		 * This method is used to abort a transfer already in progress.
		 */
		virtual void Abort () = 0;
	};

	class ITransferManager;

	struct IncomingOffer
	{
		ITransferManager *Manager_;
		uint64_t JobId_;

		QString EntryId_;

		QString Name_;
		qsizetype Size_;

		QString Description_ {};

		bool operator== (const IncomingOffer& other) const = default;
	};

	/** @brief This interface must be implemented by transfer managers
	 * returned from IAccount::GetTransferManager().
	 */
	class ITransferManager
	{
	public:
		virtual ~ITransferManager () = default;

		virtual Emitters::TransferManager& GetTransferManagerEmitter () = 0;

		/** @brief Returns whether the transfer manager is available.
		 *
		 * This method returns whether file transfers are available via
		 * the corresponding IAccount.
		 *
		 * For example, an XMPP implementation may have in-band
		 * transfers prohibited and no SOCKS server to perform the
		 * transfers out of band. In this case this method should return
		 * false, though technically XMPP supports file transfers.
		 *
		 * @return Whether files can be sent right now through this
		 * transfer manager.
		 */
		virtual bool IsAvailable () const = 0;

		virtual ITransferJob* Accept (const IncomingOffer& offer, const QString& savePath) = 0;

		virtual void Decline (const IncomingOffer&) = 0;

		/** @brief Requests a file transfer with the remote party.
		 *
		 * The entry is identified by the ID, which is the result of
		 * ICLEntry::GetEntryID().
		 *
		 * If the variant is an empty string, or there is no such
		 * variant, the file should be transferred to the variant with
		 * the highest priority.
		 *
		 * The returned object (which must implement ITransferJob)
		 * represents the file transfer request,
		 * and, further on, the file transfer job, should it be
		 * accepted.
		 *
		 * The returned object is owned by the plugin.
		 *
		 * @param[in] id The id of the remote party, as
		 * ICLEntry::GetEntryID().
		 * @param[in] variant The entry variant to send the file to.
		 * @param[in] path The path to the file that should be
		 * transferred.
		 * @param[in] comment The comment describing the file to be
		 * sent, if applicable.
		 * @return The transfer job object representing this transfer
		 * and implement ITransferJob.
		 */
		virtual ITransferJob* SendFile (const QString& id,
				const QString& variant,
				const QString& path,
				const QString& comment) = 0;
	};
}

Q_DECLARE_INTERFACE (LC::Azoth::ITransferJob, "org.Deviant.LeechCraft.Azoth.ITransferJob/1.0")
Q_DECLARE_INTERFACE (LC::Azoth::ITransferManager, "org.Deviant.LeechCraft.Azoth.ITransferManager/1.0")
