from utils.config_helper import config

class PyDB:
    def __init__(self):
        self.db_type = "postgresql" if config.database.postgresql.enabled == True else "sqlite3"

        if self.db_type == "postgresql":
            self._postgresql_init()
        elif self.db_type == "sqlite3":
            self._sqlite3_init()

        self._setup()
    
    def _postgresql_init(self):
        import psycopg2
        self.conn = psycopg2.connect(
            host=config.database.postgresql.host,
            port=config.database.postgresql.port,
            database=config.database.postgresql.database,
            user=config.database.postgresql.user,
            password=config.database.postgresql.password
        )
        self.cursor = self.conn.cursor()
    
    def _sqlite3_init(self):
        raise NotImplementedError("sqlite3 is not yet implemented")
        import sqlite3
        self.conn = sqlite3.connect(config.database.sqlite3.path)
        self.cursor = self.conn.cursor()

    # run this after creating the object to set up the database
    def _setup(self):
    # create the tables

        # create message table
        # holds timestamp and raw aprs message, must be unique
        self.cursor.execute('''
            CREATE TABLE IF NOT EXISTS messages (
                id INTEGER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                timestamp TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
                message TEXT NOT NULL UNIQUE
            );
        ''')

        # create source table
        # holds raw message id, timestamp, callsign, ssid and ip of upload source
        self.cursor.execute('''
            CREATE TABLE IF NOT EXISTS sources (
                id INTEGER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                message INTEGER NOT NULL,
                timestamp TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT CURRENT_TIMESTAMP,
                callsign TEXT NOT NULL,
                ssid INTEGER NOT NULL,
                ip INET NOT NULL,

                CONSTRAINT sources_message_fkey
                    FOREIGN KEY (message)
                    REFERENCES messages (id)
            );
        ''')

        # create telemetry table
        # holds message id, raw telemetry data, and parsed telemetry data
        self.cursor.execute('''
            CREATE TABLE IF NOT EXISTS telemetry (
                id INTEGER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                message INTEGER NOT NULL,
                raw TEXT NOT NULL,
                parsed JSONB NOT NULL,

                CONSTRAINT telemetry_message_fkey
                    FOREIGN KEY (message)
                    REFERENCES messages (id)
            );
        ''')


        # create data table
        # holds message id, callsign, ssid, symbol, speed, course, lat/lon, altitude, comment, and telemetry id
        self.cursor.execute('''
            CREATE TABLE IF NOT EXISTS data (
                id INTEGER GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
                message INTEGER NOT NULL,
                callsign TEXT NOT NULL,
                ssid INTEGER NOT NULL,
                symbol TEXT NOT NULL,
                speed REAL,
                course REAL,
                geo geography(POINT),
                altitude REAL,
                comment TEXT,
                telemetry INTEGER,

                CONSTRAINT data_message_fkey
                    FOREIGN KEY (message)
                    REFERENCES messages (id),
                CONSTRAINT data_telemetry_fkey
                    FOREIGN KEY (telemetry)
                    REFERENCES telemetry (id)
            );
        ''')

        # create device view
        # TODO

        self.conn.commit()
    

    def delete_db(self):
        # drop all tables
        
        # todo: need a safe way to do this with cascading deletes
        self.cursor.execute("DROP TABLE IF EXISTS data;")
        self.cursor.execute("DROP TABLE IF EXISTS telemetry;")
        self.cursor.execute("DROP TABLE IF EXISTS sources;")
        self.cursor.execute("DROP TABLE IF EXISTS messages;")
        self.cursor.execute("DROP TABLE IF EXISTS status;")
        self.cursor.execute("DROP TABLE IF EXISTS health;")
        self.cursor.execute("DROP TABLE IF EXISTS hostnames;")

        self.conn.commit()
    
    def add_message(self, message):
        # add message to messages table and return id
        # if message already exists, return id
        try:
            self.cursor.execute("INSERT INTO messages (message) VALUES (%s) RETURNING id;", (message,))
            self.conn.commit()
            return self.cursor.fetchone()[0]
        except:
            self.cursor.execute("SELECT id FROM messages WHERE message = %s;", (message,))
            return self.cursor.fetchone()[0]
    
    def add_source(self, message_id, timestamp, callsign, ssid, ip):
        # add source to sources table
        self.cursor.execute("INSERT INTO sources (message, timestamp, callsign, ssid, ip) VALUES (%s, %s, %s, %s, %s);", (message_id, timestamp, callsign, ssid, ip))
        self.conn.commit()
    
    def add_telemetry(self, message_id, raw, parsed):
        # add telemetry to telemetry table and return id
        self.cursor.execute("INSERT INTO telemetry (message, raw, parsed) VALUES (%s, %s, %s) RETURNING id;", (message_id, raw, parsed))
        self.conn.commit()
        return self.cursor.fetchone()[0]
    
    def add_data(self, message_id, callsign, ssid, symbol, speed, course, geo, altitude, comment, telemetry_id):
        # add data to data table
        self.cursor.execute("INSERT INTO data (message, callsign, ssid, symbol, speed, course, geo, altitude, comment, telemetry) VALUES (%s, %s, %s, %s, %s, %s, %s, %s, %s, %s);", (message_id, callsign, ssid, symbol, speed, course, geo, altitude, comment, telemetry_id))
        self.conn.commit()


def main():
    db = PyDB()