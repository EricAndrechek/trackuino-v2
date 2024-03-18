if __name__ == "__main__":
    import sys
    sys.path.append("..")

from utils.config_helper import config

host = config.database.postgre_sql.host
port = config.database.postgre_sql.port
database = config.database.postgre_sql.database
user = config.database.postgre_sql.user
password = config.database.postgre_sql.password
db_url = "postgresql://{}:{}@{}:{}/{}".format(
    user, password, host, port, database
)

import sqlalchemy as db
try:
    from sql.models import *
except ImportError:
    import sys
    sys.path.append("..")
    from sql.models import *

engine = db.create_engine(db_url, echo=False)

Base.metadata.create_all(engine)

from sqlalchemy.orm import scoped_session, sessionmaker

Session = scoped_session(sessionmaker(bind=engine))

# to update the database schema, run `alembic revision --autogenerate -m "commmit message"` and then `alembic upgrade head`
