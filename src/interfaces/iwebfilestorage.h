/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2014  Georg Rudoy
 *
 * Distributed under the Boost Software License, Version 1.0.
 * (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)
 **********************************************************************/

#pragma once

#include <QStringList>
#include <QtPlugin>

class QUrl;

/** @brief Interface for plugins supporting storing files on the web.
 *
 * Cloud storages like Google Drive and the likes also count.
 *
 * A single plugin can support multiple services, and each service can
 * be represented via multiple accounts. Each account or each accountless
 * service (like a dumb filebin) is called a <em>service variant</em>. The
 * list of all service variants is returned via GetServiceVariants()
 * method.
 *
 * Good IWebFileStorage plugins also implement IDataFilter, exposing
 * filtering entities containing URLs to local files.
 *
 * @sa IDataFilter
 */
class Q_DECL_EXPORT IWebFileStorage
{
public:
	virtual ~IWebFileStorage () {}

	/** @brief Returns the list of services supported by this plugin.
	 *
	 * @return The list of service variants.
	 */
	virtual QStringList GetServiceVariants () const = 0;

	/** @brief Uploads the given \em filename to the given \em service.
	 *
	 * This function should initiate uploading the given \em filename to
	 * the given \em service and returns immediatelly. When the file
	 * finishes uploading, the fileUploaded() signal should be emitted.
	 *
	 * The \em service is a string returned from GetServiceVariants()
	 * method. It can also be an empty string, in which case either a
	 * default service variant should be chosen, or a service choosing
	 * dialog should appear.
	 *
	 * @param[in] filename The file to upload.
	 * @param[in] service The service variant to use, which may be an
	 * empty string
	 *
	 * @sa fileUploaded()
	 * @sa GetServiceVariants()
	 */
	virtual void UploadFile (const QString& filename,
			const QString& service = QString ()) = 0;
protected:
	/** @brief Emitted when the given \em filename finishes uploading.
	 *
	 * This signal should be emitted when the given \em filename finishes
	 * uploading. The \em filename should be exactly the one passed
	 * previously to the corresponding UploadFile() call that initiated
	 * the file upload process.
	 *
	 * The URL under which the file is generally available is passed via
	 * the \em fileUrl parameter.
	 *
	 * @note This method is expected to be a signal.
	 *
	 * @param[out] filename The filename that has been just uploaded.
	 * @param[out] fileUrl The URL of under which \em filename is available.
	 *
	 * @sa UploadFile()
	 */
	virtual void fileUploaded (const QString& filename, const QUrl& fileUrl) = 0;
};

Q_DECLARE_INTERFACE (IWebFileStorage, "org.Deviant.LeechCraft.IWebFileStorage/1.0")
