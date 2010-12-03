CREATE TABLE enclosures (
    enclosure_id BIGINT PRIMARY KEY, 
    item_id BIGINT NOT NULL,
    url TEXT NOT NULL, 
    type TEXT NOT NULL, 
    length BIGINT NOT NULL, 
    lang TEXT ) Engine=InnoDB;

ALTER TABLE enclosured ADD 
  FOREIGN KEY ( item_id ) 
    REFERENCES items ( item_id )
      ON DELETE CASCADE 
      ON UPDATE CASCADE ;
