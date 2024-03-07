import gps
import time
import threading

class GPS():
    def __init__(self):
        self.gpsd = gps.gps(mode=gps.WATCH_ENABLE)
        self.data = None
        self.thread = threading.Thread(target=self.update, daemon=True)
        self.thread.start()

    def update(self):
        nx = self.gpsd.next()
