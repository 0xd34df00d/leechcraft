UPDATE channels
    SET description =  ? , last_build = ? , tags =  ? , language = ? ,
        author = ? , pixmap_url = ? , pixmap = ? , favicon =  ? , display_title = ?
    WHERE channel_id = ? ;
