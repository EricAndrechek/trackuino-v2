from redis import Redis
from rq import Queue
from rq_scheduler import Scheduler
from datetime import datetime
from tasks.update_prediction import update_prediction


queue = Queue('bar', connection=Redis())
scheduler = Scheduler(queue=queue, connection=queue.connection)

# Puts a job into the scheduler. The API is similar to RQ except that it
# takes a datetime object as first argument. So for example to schedule a
# job to run on Jan 1st 2020 we do:
scheduler.enqueue_at(datetime(2020, 1, 1), func) # Date time should be in UTC

# Here's another example scheduling a job to run at a specific date and time (in UTC),
# complete with args and kwargs.
scheduler.enqueue_at(datetime(2020, 1, 1, 3, 4), func, foo, bar=baz)

# You can choose the queue type where jobs will be enqueued by passing the name of the type to the scheduler
# used to enqueue
scheduler = Scheduler('foo', queue_class="rq.Queue")
scheduler.enqueue_at(datetime(2020, 1, 1), func) # The job will be enqueued at the queue named "foo" using the queue type "rq.Queue"