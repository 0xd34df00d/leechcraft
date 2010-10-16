CREATE TABLE enclosures (
    enclosure_id BIGINT PRIMARY KEY, 
    item_id BIGINT NOT NULL REFERENCES items ON DELETE CASCADE,
    url TEXT NOT NULL, 
    type TEXT NOT NULL, 
    length BIGINT NOT NULL, 
    lang TEXT ) Engine=InnoDB;
