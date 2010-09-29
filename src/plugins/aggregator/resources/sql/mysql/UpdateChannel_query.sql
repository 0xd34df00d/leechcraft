UPDATE channels 
    SET description =  ? , last_build = ? , tags =  ? , language = ? , 
        author = ? , pixmap_url = ? , pixmap = ? , favicon =  ? 
    WHERE channel_id = ? ;
