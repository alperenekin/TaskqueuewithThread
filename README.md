# TaskqueuewithThread
  Pthreads program that implements a “task queue.”. The main thread generates tasks to be carried out by the other threads. 
  A task is either an insert, adelete, or a search a value in a sorted list (in ascending order) with no duplicates. Each time the
  main thread generates a new task by adding it to a task queue, it awakens a thread with a condition
  signal. When a thread is awakened, it pulls a task from the queue and processes it. When a thread
  finishes executing its task, it should return to a condition wait. When the main thread completes
  generating tasks, it sets a global variable indicating that there will be no more tasks, and awakens all
  the threads with a condition broadcast. Pthread threads,mutexes and conditions are used for this system.
 ## Installation and Setup Instructions

Clown the C source code and compile it. 
### About Project

This project is written to give an idea about thereads,mutexes and conditions.
