# this is based on: https://github.com/aaronjridley/Balloons

# as it involves a lot of computation, it is run as a separate task with redis

# this task is run every 5 minutes for every active balloon

# it is run by the scheduler in backend/tasks/scheduler.py

def update_prediction(callsign, ssid)