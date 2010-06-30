CREATE TABLE infos (
	package_id INTEGER REFERENCES packages ON DELETE CASCADE,
	short_descr TEXT NOT NULL,
	long_descr TEXT,
	maint_name TEXT NOT NULL,
	maint_email TEXT NOT NULL,
	icon_url TEXT
);