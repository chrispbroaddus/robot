CREATE TABLE users (
    id UUID PRIMARY KEY,
    user_name VARCHAR(75) NOT NULL
);

/* users_auth is one to one to users */
CREATE TABLE users_auth (
    user_id UUID REFERENCES users(id),
    password VARCHAR(75) NOT NULL,
    is_admin BOOLEAN DEFAULT false,
    permission INTEGER DEFAULT 1
);

CREATE TABLE vehicles (
    id VARCHAR(250) PRIMARY KEY NOT NULL
);

/* vehicles_auth is a one to many with vehicles as vehicles can have multiple tokens */
CREATE TABLE vehicles_auth (
    vehicle_id VARCHAR(250) REFERENCES vehicles(id),
    token VARCHAR(250) NOT NULL,
    token_state INTEGER DEFAULT 0,
    created_at TIMESTAMPTZ DEFAULT NOW()
);

CREATE TABLE vehicles_locations (
    vehicle_id VARCHAR(250) REFERENCES vehicles(id),
    time_stamp TIMESTAMPTZ DEFAULT NOW(),
    lat FLOAT NOT NULL,
    lon FLOAT NOT NULL,
    alt FLOAT NOT NULL
);

CREATE TABLE vehicles_cameras (
    vehicle_id VARCHAR(250) REFERENCES vehicles(id),
    id VARCHAR(100) NOT NULL,
    serial_num INTEGER,
    width INTEGER,
    height INTEGER 
);