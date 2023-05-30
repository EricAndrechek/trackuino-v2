import sqlite3

class PyDB:
    def __init__(self, db_name='../database.db'):
        self.conn = sqlite3.connect(db_name)
        self.cursor = self.conn.cursor()

        self._setup()

    # run this after creating the object to set up the database
    def _setup(self):
        # check if the database is empty
        self.cursor.execute("SELECT name FROM sqlite_master WHERE type='table';")
        tables = self.cursor.fetchall()
        if len(tables) == 0:
            # create the tables

            # create status monitoring table
            self.cursor.execute("CREATE TABLE STATUS (ID INTEGER PRIMARY KEY, TIMESTAMP INTEGER, HOST INTEGER, HEALTH INTEGER, CONNECTIONS INTEGER);")
            
            # create host reference tables
            self.cursor.execute("CREATE TABLE HOSTNAMES (ID INTEGER PRIMARY KEY, VALUE INTEGER, NAME TEXT, HOSTNAME TEXT);")
            self.cursor.execute("CREATE TABLE HEALTH (ID INTEGER PRIMARY KEY, VALUE INTEGER, NAME TEXT, DESCRIPTION, TEXT);")

            self.conn.commit()

    def execute(self, sql):
        self.cursor.execute(sql)
        self.conn.commit()

    def fetchall(self, sql):
        self.cursor.execute(sql)
        return self.cursor.fetchall()

    def fetchone(self, sql):
        self.cursor.execute(sql)
        return self.cursor.fetchone()

    def close(self):
        self.cursor.close()
        self.conn.close()


if __name__ == '__main__':
    db = PyDB()