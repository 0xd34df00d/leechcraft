CREATE TABLE infos (
	name TEXT NOT NULL UNIQUE,
	short_descr TEXT NOT NULL,
	long_descr TEXT,
	type SMALLINT NOT NULL,
	language TEXT,
	maint_name TEXT NOT NULL,
	maint_email TEXT NOT NULL,
	icon_url TEXT
);