import multiprocessing
from utils import config_helper

wsgi_app = "app:app"
bind = f'{config_helper.config.host}:{config_helper.config.port}' or "127.0.0.1:5000"
workers = multiprocessing.cpu_count() * 2 + 1
worker_class = "gevent"

loglevel = "debug" if config_helper.config.debug else "info"

reload = config_helper.config.debug

accesslog = "access.log"
access_log_format = "%(t)s %({x-forwarded-for}i)s %(l)s %(r)s %(s)s, %(b)s bytes in %(L)s seconds, %(f)s %(a)s"

errorlog = "error.log"
capture_output = True

