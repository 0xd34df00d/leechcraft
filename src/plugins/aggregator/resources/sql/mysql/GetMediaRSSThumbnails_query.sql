SELECT mrss_thumb_id, url, width, height, time 
    FROM mrss_thumbnails WHERE mrss_id = ? ORDER BY time
