SELECT channel_id, title, display_title, url, tags, last_build, favicon, author
    FROM channels
        WHERE feed_id = ?
            ORDER BY title;
