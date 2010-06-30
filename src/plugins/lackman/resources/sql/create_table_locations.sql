CREATE TABLE locations (
	package_id INTEGER REFERENCES packages,
	component_id INTEGER REFERENCES components,
	dirname TEXT
);