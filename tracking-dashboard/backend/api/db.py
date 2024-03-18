from flask import Blueprint

db_app = Blueprint('db_app', __name__)

# database endpoints

@db_app.route('/db', methods=['DELETE'])
def api_db_delete():
    # call the delete method of the db class
    # TODO: implement delete method (need to figure out context of db class) <-- I don't remember what I meant when I wrote this...
    # TODO: need authorization
    return "not implemented", 501

@db_app.route('/db', methods=['GET'])
def api_db_get():
    # get all data from the db and return the files as a downloadable zip
    # TODO: need to implement, probably need to run as a background job...
    return "not implemented", 501

# TODO: allow getting specific tables from the db and render them in the browser
# maybe just redirect to pgadmin instead?
