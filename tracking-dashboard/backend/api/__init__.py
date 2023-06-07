from flask import Blueprint

api_bp = Blueprint('api_bp', __name__)

from api.status import status_app
from api.map import map_app
from api.db import db_app
from api.server import server_app
from api.data import data_app

api_bp.register_blueprint(status_app)
api_bp.register_blueprint(map_app)
api_bp.register_blueprint(db_app)
api_bp.register_blueprint(server_app)
api_bp.register_blueprint(data_app)

