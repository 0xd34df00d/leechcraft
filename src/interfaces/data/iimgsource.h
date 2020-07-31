/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QtPlugin>
#include <QUrl>
#include <QString>
#include <QList>
#include <QSize>

namespace LC
{
	/** @brief Describes a single image hosting service (including account).
	 *
	 * If a service supports multiple accounts, distinct accounts should
	 * correspond to distinct ImageServiceInfo structures.
	 */
	struct ImageServiceInfo
	{
		/** @brief The unique ID of the service (including account ID).
		 */
		QByteArray ID_;

		/** @brief The human-readable name of the service.
		 */
		QString Name_;
	};

	/** @brief The list of image storage service descriptions.
	 */
	typedef QList<ImageServiceInfo> ImageServiceInfos_t;

	/** @brief Describes a remote image.
	 */
	struct RemoteImageInfo
	{
		/** @brief The link to the full-sized image.
		 */
		QUrl Full_;

		/** @brief The size of the full-sized image, if known.
		 */
		QSize FullSize_;

		/** @brief The link to the preview version of the image, if
		 * applicable.
		 */
		QUrl Preview_;

		/** @brief The size of the preview version of the image, if
		 * applicable.
		 */
		QSize PreviewSize_;

		/** @brief The thumbnail-sized version of the image, if
		 * applicable.
		 *
		 * This version should generally be smaller than the preview
		 * version.
		 */
		QUrl Thumb_;

		/** @brief The size of the thumbnail-sized version of the image,
		 * if applicable.
		 */
		QSize ThumbSize_;

		/** @brief The title of the image, if known.
		 */
		QString Title_;
	};

	/** @brief A list of remote images.
	 */
	typedef QList<RemoteImageInfo> RemoteImageInfos_t;
}

/** @brief Pending image request proxy object.
 *
 * This class is used to notify the IImgSource's methods caller when the
 * information about requested images becomes available.
 *
 * The class is self-owned, that is, it is deleted as soon as the control
 * gets back to the event loop after emitting its ready() or error()
 * signals.
 */
class IPendingImgSourceRequest
{
public:
	virtual ~IPendingImgSourceRequest () {}

	/** @brief Returns this object as QObject.
	 *
	 * This function returns this object as a pointer to QObject to allow
	 * connecting to its signals.
	 *
	 * @return This object as a QObject.
	 */
	virtual QObject* GetQObject () = 0;

	/** @brief Returns the information about the selected images.
	 *
	 * This method will very likely return an empty list until the
	 * ready() signal is emitted.
	 *
	 * @return The information about the selected images.
	 */
	virtual LC::RemoteImageInfos_t GetInfos () const = 0;
protected:
	/** @brief Emitted when the information about the requested images
	 * becomes available.
	 *
	 * Until this signal is emitted, the GetInfos() method very likely
	 * returns an empty list.
	 *
	 * The object is deleted after emitting this signal after the control
	 * gets back to the event loop.
	 */
	virtual void ready () = 0;

	/** @brief Emitted if there is an error obtaining information about
	 * the requested images.
	 *
	 * The object is deleted after emitting this signal after the control
	 * gets back to the event loop.
	 *
	 * @note The ready() signal is not emitted after this one.
	 *
	 * @param[out] text The human-readable text about the error.
	 */
	virtual void error (const QString& text) = 0;
};

/** @brief Interface for remote image storage plugins.
 *
 * This interface should be implemented by plugins that provide a
 * read-only interface to images hosted on a remote service, like
 * Picasa or Flickr.
 */
class IImgSource
{
public:
	virtual ~IImgSource () {}

	/** @brief Returns the list of supported services.
	 *
	 * @return The list of supported image services.
	 *
	 * @sa LC::ImageServiceInfo
	 */
	virtual LC::ImageServiceInfos_t GetServices () const = 0;

	/** @brief Requests the images for the given service.
	 *
	 * The implementation may either return all the images for the given
	 * \em serviceId or start the IImgSource-implementing plugin's default
	 * image chooser dialog, focused on the given service.
	 *
	 * A pending request proxy object is returned. When the user selects
	 * the desired images and accepts the dialog, the proxy object emits
	 * the IPendingImgSourceRequest::ready() signal, and the images are
	 * available via IPendingImgSourceRequest::GetInfos(). If the user
	 * rejects the dialog, the signal is still emitted, but the
	 * IPendingImgSourceRequest::GetInfos() method returns an empty list.
	 *
	 * @param[in] serviceId The ID of a service returned from GetServices().
	 * @return The pending request proxy object.
	 */
	virtual IPendingImgSourceRequest* RequestImages (const QByteArray& serviceId) = 0;

	/** @brief Requests the default image chooser to be opened.
	 *
	 * This function opens the IImgSource-implementing plugin's default
	 * image chooser dialog, where the user can select arbitrary images
	 * from arbitrary services.
	 *
	 * The semantics of the returned pending proxy object is the same as
	 * in the RequestImages() method.
	 *
	 * @return The pending request proxy object.
	 */
	virtual IPendingImgSourceRequest* StartDefaultChooser () = 0;
};

Q_DECLARE_INTERFACE (IPendingImgSourceRequest, "org.Deviant.LeechCraft.IPendingImgSourceRequest/1.0")
Q_DECLARE_INTERFACE (IImgSource, "org.Deviant.LeechCraft.IImgSource/1.0")
