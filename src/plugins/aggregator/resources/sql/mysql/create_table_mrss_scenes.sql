CREATE TABLE mrss_scenes (
    mrss_scene_id BIGINT PRIMARY KEY, 
    mrss_id BIGINT NOT NULL, 
    title TEXT, 
    description TEXT, 
    start_time TEXT, 
    end_time TEXT
);

ALTER TABLE mrss_scenes ADD 
  FOREIGN KEY ( mrss_id ) 
    REFERENCES mrss ( mrss_id )
      ON DELETE CASCADE 
      ON UPDATE CASCADE ;
