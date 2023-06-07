from flask import Blueprint

db_app = Blueprint('db_app', __name__)

# database endpoints

@db_app.route('/db', methods=['DELETE'])
def api_db_delete():
    # TODO: call the delete method of the db class (need to figure out context of db class)
    return "not implemented", 501

@db_app.route('/db', methods=['GET'])
def api_db_get():
    # TODO: get the current db file and return it
    return "not implemented", 501

# TODO: allow getting specific tables from the db and render them in the browser
