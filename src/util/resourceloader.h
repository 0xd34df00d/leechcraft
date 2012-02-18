/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2012  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#ifndef UTIL_RESOURCELOADER_H
#define UTIL_RESOURCELOADER_H
#include <memory>
#include <QObject>
#include <QStringList>
#include <QDir>
#include <QCache>
#include "utilconfig.h"

class QAbstractItemModel;
class QStandardItemModel;
class QSortFilterProxyModel;
class QFileSystemWatcher;
class QTimer;
class QPixmap;

namespace LeechCraft
{
	namespace Util
	{
		typedef std::shared_ptr<QIODevice> QIODevice_ptr;

		/** @brief Utility class for loading a file from a set of locations.
		 *
		 * One may want to load a resource file from a set of locations,
		 * trying them in order and sticking with the first found file.
		 * There may be different sets of resources (icon packs, theme
		 * packs, etc.) with the same layout of files in them, and one
		 * may want to easily load a file from the currently selected
		 * set. For example, a theme file may be first searched for in
		 * user's home directory, then in some global place, and maybe
		 * even somewhere else as well.
		 *
		 * This class provides means to make this task easy. The
		 * constructor takes a relative path from each location that
		 * should be looked for. AddGlobalPrefix() function registers
		 * platform-specific search paths (like ones in /usr/share on
		 * UNIX and relative to main application binary on Windows).
		 * AddLocalPrefix() registers paths relatively to the
		 * ~/.leechcraft/data/ path. Then GetPath() function is used to
		 * get a full path to the corresponding object or Load() for
		 * loading that object. Prefixes are searched for in reverse
		 * order, from last registered to first one.
		 *
		 * A model suitable for using in XmlSettingsDialog is also
		 * provided, which watches for different subdirectories in
		 * registered paths and is handy for supporting different sets
		 * of resources.
		 *
		 * The class also supports filtering its results, filters may be
		 * set with SetAttrFilters() and SetNameFilters().
		 *
		 * All registered paths are watched for changes, so, for
		 * example, installation of new icon packs would require no
		 * actions from both user (to make LeechCraft fetch newly
		 * installed data) and programmer (to support this feature).
		 *
		 * @note One should make sure that all created instances of this
		 * class are deleted in IInfo::Release() or earlier, otherwise
		 * deadlocks in directory watcher may prevent LeechCrat from
		 * shutting down completely.
		 */
		class UTIL_API ResourceLoader : public QObject
		{
			Q_OBJECT

			QStringList LocalPrefixesChain_;
			QStringList GlobalPrefixesChain_;
			QString RelativePath_;

			QStandardItemModel *SubElemModel_;
			QStringList NameFilters_;
			QDir::Filters AttrFilters_;
			QSortFilterProxyModel *SortModel_;

			QFileSystemWatcher *Watcher_;

			QTimer *CacheFlushTimer_;
			mutable QCache<QString, QByteArray> CachePathContents_;
		public:
			/** @brief Initializes the loader with the given path.
			 *
			 * @param[in] relPath Path relative to registered prefixes
			 * where the resources will be searched for.
			 * @param[in] obj Parent object.
			 */
			ResourceLoader (const QString& relPath, QObject* obj = 0);

			/** @brief Registers global OS-dependent prefixes.
			 *
			 * Registers global prefixes as search pathes. It's
			 * /usr/local/share/leechcraft/ and /usr/share/leechcraft/
			 * on UNIX, share/ in the directory with the main executable
			 * on Windows and ../Resources/ dir relative to application
			 * path.
			 */
			void AddGlobalPrefix ();

			/** @brief Registers a local search prefix.
			 *
			 * The prefix string is relative to ~/.leechcraft/data. It
			 * may have several directories in the path, like
			 * "plugin/directory/subdir".
			 */
			void AddLocalPrefix (QString prefix = QString ());

			/** @brief Sets the caching parameters of this loader.
			 *
			 * This function sets the size of the cache in kibibytes
			 * and the timeout between cache flushes in milliseconds.
			 * That's it, the cache would be flushed each timeout
			 * milliseconds automatically. If timeout is less than 0,
			 * then automatic cache flushing is disabled.
			 *
			 * @param[in] size The size of the cache in kibibytes.
			 * @param[in] timeout The timeout between cache flushes.
			 *
			 * @sa FlushCache()
			 */
			void SetCacheParams (int size, int timeout);

			/** @brief Forcefully flushes the cache.
			 *
			 * This function may be used to flush the contents of this
			 * resource loader cache. It may be useful, for example,
			 * when the resources have been updated on disk, and the
			 * new versions should be updated right now.
			 */
			void FlushCache ();

			/** @brief Lists the available files for the given option.
			 *
			 * The option should be the one of the elements of the model
			 * returned by GetSubElemModel().
			 *
			 * This method supports basic filtering: list of name
			 * filters could be passed via the names parameter, and
			 * additional filter flags via the filters parameter. The
			 * default is to perform no additional filtering.
			 *
			 * @param[in] option The option for which to list available
			 * files/directories.
			 * @param[in] names Name filters.
			 * @param[in] filters Additional filters.
			 */
			QFileInfoList List (const QString& option,
					const QStringList& names = QStringList (),
					QDir::Filters filters = QDir::NoFilter) const;

			/** @brief Returns the first found path for the list of variants.
			 *
			 * For each registered prefix this function iterates over
			 * pathVariants list and checks if the file at
			 * $prefix/$relPath/$pathVariant exists, and if it does, the
			 * full path to this file is returned. Here, relPath is the
			 * path passed to the constructor.
			 *
			 * pathVariants may be useful if there may be several
			 * representations of each resource. For example, an icon can
			 * be in jpg, png or svg format, thus pathVariants would
			 * contain something like:
			 * - themename/iconname.svg
			 * - themename/iconname.png
			 * - themename/iconname.jpg
			 * Here again, path variants are tried from first to last.
			 *
			 * If nothing is found, a null string is returned.
			 *
			 * @param[in] pathVariants The list of variants to try.
			 * @return The first found full path or null string.
			 *
			 * @sa Load(), GetIconPath()
			 */
			QString GetPath (const QStringList& pathVariants) const;

			/** @brief Calls GetPath() with standard variants for the
			 * icon extensions.
			 *
			 * This functions just calls the GetPath() function with
			 * standard icon extensions, that is, the path variants are
			 * basename + (".svg", ".png", ".jpg")
			 *
			 * @note If possible, consider using LoadIcon() or
			 * LoadPixmap() functions and enabling the cache. That may
			 * speed things up considerably.
			 *
			 * @param[in] basename Base name of the icon — without
			 * extension.
			 * @return The first found full path or null string
			 *
			 * @sa GetPath(), LoadIcon()
			 */
			QString GetIconPath (const QString& basename) const;

			/** @brief Returns the QIODevice for the corresponding resource.
			 *
			 * This function behaves exactly like GetPath(), except it
			 * creates a QIODevice for the found path and opens it.
			 *
			 * @param[in] pathVariants The list of variants to try.
			 * @param[in] open Whether the returned QIODevice should be
			 * opened automatically.
			 * @return The QIODevice for the found path, or a null ptr
			 * if nothing is found.
			 */
			QIODevice_ptr Load (const QStringList& pathVariants, bool open = false) const;

			/** @brief Returns the QIODevice for the corresponding resource.
			 *
			 * This function is the overloaded variant of Load(),
			 * provided for convenience. This function constructs a
			 * stringlist of a single element, the passed pathVariant,
			 * and calls the other Load() with it.
			 *
			 * @param[in] pathVariant The path variant to try.
			 * @param[in] open Whether the returned QIODevice should be
			 * opened automatically.
			 * @return The QIODevice for the found path, or a null ptr
			 * if nothing is found.
			 */
			QIODevice_ptr Load (const QString& pathVariant, bool open = false) const;

			/** @brief Returns the QIODevice for the corresponding icon.
			 *
			 * This function is analogous to GetIconPath().
			 *
			 * @param[in] basename Base name of the icon — without
			 * extension.
			 * @param[in] open Whether the returned QIODevice should be
			 * opened automatically.
			 * @return The QIODevice for the found path, or a null ptr
			 * if nothing is found.
			 *
			 * @sa GetIconPath(), LoadPixmap()
			 */
			QIODevice_ptr LoadIcon (const QString& basename, bool open = false) const;

			/** @brief Returns the pixmap for the given basename.
			 *
			 * This function is analogous to GetIconPath(), but instead
			 * of returning the path, it reads the pixmap at that path and
			 * returns the pixmap.
			 *
			 * If there is no such path, or the pixmap failed to read, a
			 * null pixmap is returned.
			 *
			 * @param[in] basename Base name of the pixmap — without
			 * extension.
			 * @return The QPixmap from the found path, or a null pixmap
			 * if nothing is found, or if pixmap parsing is failed.
			 *
			 * @sa GetIconPath(), LoadIcon()
			 */
			QPixmap LoadPixmap (const QString& basename) const;

			/** @brief Returns the subelement model with the contents of registered paths.
			 *
			 * For each prefix in the list of registered prefixes, the
			 * contents of $prefix/$relPath are listed in this model,
			 * where relPath is the path passed to the constructor. The
			 * results are filtered according to the filters set with
			 * SetAttrFilters() and SetNameFilters().
			 *
			 * This model may be used in settings dialogs as a data
			 * source, which may be handy when one wants to implement
			 * support for iconsets, theme packs and such things in his
			 * plugin.
			 *
			 * @return The model listing the subelements of registered paths.
			 *
			 * @sa SetAttrFilters(), SetNameFilters().
			 */
			QAbstractItemModel* GetSubElemModel () const;

			/** @brief Sets the attribute filters for the subelement model.
			 *
			 * This is analogous to QDir's functions for filtering its
			 * results.
			 *
			 * By default,
			 * QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable
			 * is set.
			 *
			 * @note This function should be called before any paths are
			 * registered in this resource loader, otherwise unwanted
			 * results may occur in the subelement model.
			 *
			 * @sa SetNameFilters(), GetSubElemModel()
			 */
			void SetAttrFilters (QDir::Filters);

			/** @brief Sets the name filters for the subelement model.
			 *
			 * Wildcards may be used in name fitlers. Use an empty
			 * QStringList to disable filtering. By default, no filter is
			 * set.
			 *
			 * This is analogous to QDir's functions for filtering its
			 * results.
			 *
			 * @note This function should be called before any paths are
			 * registered in this resource loader, otherwise unwanted
			 * results may occur in the subelement model.
			 *
			 * @sa SetAttrFilters(), GetSubElemModel()
			 */
			void SetNameFilters (const QStringList&);
		private:
			void ScanPath (const QString&);
		private slots:
			void handleDirectoryChanged (const QString&);
			void handleFlushCaches ();
		signals:
			void watchedDirectoriesChanged ();
		};
	}
}

#endif
