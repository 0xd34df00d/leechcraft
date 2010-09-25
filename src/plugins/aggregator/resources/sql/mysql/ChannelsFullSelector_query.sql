SELECT url, title, description, last_build, tags, language, author, pixmap_url, pixmap, favicon 
    FROM channels 
        WHERE channel_id = ? 
            ORDER BY title;
