SELECT mrss_scene_id, title, description, start_time, end_time 
    FROM mrss_scenes 
        WHERE mrss_id = ? 
            ORDER BY start_time
