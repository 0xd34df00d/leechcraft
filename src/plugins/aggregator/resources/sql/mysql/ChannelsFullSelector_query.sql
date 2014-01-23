SELECT url, title, description, last_build, tags, language, author, pixmap_url, pixmap, favicon, display_title
    FROM channels
        WHERE channel_id = ?
            ORDER BY title;
