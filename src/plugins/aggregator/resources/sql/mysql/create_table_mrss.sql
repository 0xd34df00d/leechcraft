CREATE TABLE mrss (
    mrss_id BIGINT PRIMARY KEY, 
    item_id BIGINT NOT NULL, 
    url TEXT, 
    size BIGINT, 
    type TEXT, 
    medium TEXT, 
    is_default TINYINT, 
    expression TEXT, 
    bitrate INTEGER, 
    framerate REAL, 
    samplingrate REAL, 
    channels SMALLINT, 
    duration INTEGER, 
    width INTEGER, 
    height INTEGER, 
    lang TEXT, 
    mediagroup INTEGER, 
    rating TEXT, 
    rating_scheme TEXT, 
    title TEXT, 
    description TEXT, 
    keywords TEXT, 
    copyright_url TEXT, 
    copyright_text TEXT, 
    star_rating_average SMALLINT, 
    star_rating_count INTEGER, 
    star_rating_min SMALLINT, 
    star_rating_max SMALLINT, 
    stat_views INTEGER, 
    stat_favs INTEGER, 
    tags TEXT, 
    item_parents_hash TEXT, 
    item_title TEXT, 
    item_url TEXT
) Engine=InnoDB;

ALTER TABLE mrss ADD 
  FOREIGN KEY ( item_id ) 
    REFERENCES items ( item_id )
      ON DELETE CASCADE 
      ON UPDATE CASCADE ;
