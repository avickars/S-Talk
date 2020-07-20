# CMPT 300, Assignment3


## Notes
* Running Valgrind will leave 4 memory leaks that appear to be a result of pthread_cancell().  Per Dr. Fraser's postings this is acceptable.
* You will notice that each time a thread does a wait, a boolean variable "waiting" is set to true (and set to false when the thread finishes waiting).  This is done to ensure that any thread that is cancelled while on a wait gives of the mutex (see the cleanup method for each thread).
* In sender.c you will notice that each message that is received as input from the user is copied to a char array before being sent.  This is because sending the 