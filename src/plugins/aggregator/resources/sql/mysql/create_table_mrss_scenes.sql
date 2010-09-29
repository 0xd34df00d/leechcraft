CREATE TABLE mrss_scenes (
    mrss_scene_id BIGINT PRIMARY KEY, 
    mrss_id BIGINT NOT NULL REFERENCES mrss ON DELETE CASCADE, 
    title TEXT, 
    description TEXT, 
    start_time TEXT, 
    end_time TEXT
);

