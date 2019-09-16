/* File:     
 *     taskqueue.c
 *
 * Purpose:  
 *     Implement a task queue using pthreads.
 *     The main thread begins by starting a user-specified
 *     number of threads that immediately go to sleep in a 
 *     condition wait. The main thread generates new tasks
 *     to be carried out by other threads. When main thread
 *     completes generating tasks, it sets a global variable
 *     to indicate no more tasks and awakens all threads with
 *     a condition broadcast.
 *
 * Input:
 *     number of threads
 *     number of tasks to be generated
 *
 * Notes:  
 *     -  tasks are Linked list operations
 *     -  task options:   
 *        0: insert
 *        1: delete
 *        2: check if the data is in the list
 *        default: print the Linkedlist
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#define BILLION 1E9
const int max_title = 1000;
const int task_count = 3;
const int max_val = 50;
int a =1;
pthread_mutex_t lock;
pthread_mutex_t lock2;
pthread_cond_t cond_var=PTHREAD_COND_INITIALIZER;
int flag=0;
/* Struct for list nodes */
struct list_node_s {
   int    data;
   struct list_node_s* next;
};

/* Struct for task nodes */
struct task_node_s {
   int which_task;
   int option;
   int data;
   struct task_node_s* next;
};

/* Shared variables */
int thread_count;



/* Head of linked list */
struct list_node_s* head = NULL;  

/* Head of task queue */
struct task_node_s* tasks_head = NULL;

/* Tail of task queue */	
struct task_node_s* tasks_tail = NULL;

/* Usage */
void Usage(char* prog_name);

/* Thread function */
void* Thread_work(void* rank);

/* Task queue functions */
int Empty_queue(void);
int Terminate(long my_rank, int* which_task_p, int* option_p, int* data_p);
void Task_queue(int n);
void Task_enqueue(int which_task, int option, int data);
int Task_dequeue(long my_rank, int* which_task_p, int* option_p, int* data_p);

/* List operations */
int Insert(int value);
void Print(char title[]);
int Search(int value);
int Delete(int value);
void Free_list(void);
int Is_empty(void);

/*-----------------------------------------------------------------*/
int main(int argc, char* argv[]) {
   long thread;
clock_t begin = clock(); //start the time
   pthread_t* thread_handles;
   int n;
   int i=0;
   
   if(argc != 3) Usage(argv[0]); // input sayısı yeterli değilse buraya gir.
   
   thread_count = strtol(argv[1], NULL, 10);
   n = strtol(argv[2], NULL, 10);
   
   /* Allocate array for threads */
   pthread_t tid[thread_count];

   
   /* Initialize mutexes and condition variables */
   if(pthread_mutex_init(&lock, NULL) != 0)
	{
		printf("ERROR: Cannot initiate mutex\n");
		return -1;
	}
  
   /* Start threads with their thread number */
   for(i=0; i<thread_count; i++) {
		int* p = malloc(sizeof(int));
		*p=i; 
	pthread_create(&tid[i], NULL, Thread_work,p);
   }
  
       
   
   /* Generate tasks */
   Task_queue(n);
      
   		

   /* Wait for threads to complete */
   for(i=0; i<thread_count; i++) {
	pthread_join(tid[i], NULL);
   }

/* destroy all mutexes and conditional variables */
pthread_mutex_destroy(&lock);
pthread_mutex_destroy(&lock2);
pthread_cond_destroy(&cond_var);   
   Free_list();
clock_t end = clock(); //end of the time 
double time_spent = (double)(end - begin) / CLOCKS_PER_SEC; //calculate the time spent
printf("Total time spent %lf, for %d threads\n", time_spent, thread_count);
   
   return 0;
}  /* main */


/*-------------------------------------------------------------------
 * Function:    Task_queue
 * Purpose:     generate random tasks for the task queue, and 
 *              notify a thread to wake up from condition 
 *              wait and get a task
 * In arg:      n: number of tasks
 * Global var:  queue_mutex, cond_mutex, cond_queue, cond_task
 *              thread_cond_wait, thread_count, flag
 */

void Task_queue(int n) {
   int i;
/*Some synch code here!!!*/
	
   for(i = 0; i < n; i++) {
      
      Task_enqueue(i, random() % task_count, random() % max_val); // Create n tasks
      pthread_cond_signal(&cond_var);	//send signal to threads to awake them one by one 
	
     
   }
int b=Empty_queue();

while(b==0){ //If there are still tasks left send broadcast signal to wake them up all together
	b=Empty_queue();
	if(b==1){ // when the queue is empty stop sending signal end break.
		a=0;
		break;
	}
	pthread_cond_broadcast(&cond_var); 

}   

   
   
/* Now the queue is empty:  wait for threads to terminate */

pthread_cond_broadcast(&cond_var); 

   Print("main:  Final list");

}  /* Task_queue */


/*-------------------------------------------------------------------
 * Function:    Empty_queue
 * Purpose:     Determine whether the task queue is empty
 * Return val:  0 task queue not empty
 *              1 otherwise
 */
int Empty_queue(void) {
   if (tasks_head == NULL)
      return 1;
   else
      return 0;
}  /* Empty_queue */


/*-------------------------------------------------------------------
 * Function:    Task_enqueue
 * Purpose:     insert a new task into task queue
 * In arg:      option, data
 * Global var:  tasks_head, tasks_tail
 */
/*NOTHING TO CHANGE HERE*/
void Task_enqueue(int which_task, int option, int data){
   struct task_node_s* tmp_task = NULL;
   
   tmp_task = malloc(sizeof(struct task_node_s));
   tmp_task->which_task = which_task;
   tmp_task->option = option;
   tmp_task->data = data;
   tmp_task->next = NULL;
   
   if (tasks_tail == NULL) { //task list is empty   
      tasks_head = tmp_task;
      tasks_tail = tmp_task;
   } else {
      tasks_tail->next = tmp_task;
      tasks_tail = tmp_task;
   }

# ifdef DEBUG
   switch(option) {
      case 0:
         printf("Main:  enqueued task %d: Insert %d\n", which_task, data);
         break;
      case 1:
         printf("Main:  enqueued task %d: Delete %d\n", which_task, data);
         break;
      case 2:
         printf("Main:  enqueued task %d: Search %d\n", which_task, data);
         break;
      default:
         printf("Main:  enqueued task %d: Print list\n", which_task);
   }
#endif
   
}  /* Task_enqueue */


/*-------------------------------------------------------------------
 * Function:    Task_dequeue
 * Purpose:     take a task from task queue
 * In arg:      my_rank
 * Out arg:     which_task_p, option_p, data_p
 * Global var:  tasks_head, tasks_tail
 * Return val:  0 if queue is empty, 1 otherwise
 */
/*NOTHING TO CHANGE HERE*/
int Task_dequeue(long my_rank, int* which_task_p, int* option_p, 
      int* data_p){
   struct task_node_s* tmp_tasks_head = tasks_head;
   
   if (tmp_tasks_head == NULL) {
      printf("Th %ld > Queue empty\n", my_rank);
      return 0;
   }
   
   *which_task_p = tmp_tasks_head->which_task;  //WHICH TASK = TASK NO
   *option_p = tmp_tasks_head->option;
   *data_p = tmp_tasks_head->data;
   
   if (tasks_tail == tasks_head) //last task
      tasks_tail = tasks_tail->next;
   
   tasks_head = tasks_head->next;
   free(tmp_tasks_head);
   
   return 1;
}  /* Task_dequeue */

/*-------------------------------------------------------------------  
 * Function:    Thread_work
 * Purpose:     When main thread signals a thread
 *              carry out a linked list operation
 * In arg:      rank
 * Global var:  list_mutex
 */
/*Some synch code here!!!*/
void *Thread_work(void* rank) {
while(a==1){ // stay in the loop till there is no task left
   long my_rank = *((long*) rank); //THREAD number
   char title[max_title];

   int option = 0, data = 0, which_task;	
   pthread_mutex_lock(&lock);
   pthread_cond_wait(&cond_var,&lock); // wait for the signal.It provides us to stay in wait condition after executings threads.So basically it sleeps threads.
   pthread_mutex_unlock(&lock);
if(Empty_queue() && flag==1){ //It determines if there is no task left and it guarantees that the threads worked at least one.Because the queue is also empty in the beginning.Flag helps us to determine it.
	return NULL;
}

   Task_dequeue(my_rank,&which_task,&option,&data);// Take a task from queue
        
/* lock 2 prevent race condition ,so there is no chance 2 threads to make same operation */
      switch (option) {
         case 0:
	pthread_mutex_lock(&lock2);
            if (Insert(data))
               printf("Thread %ld: task %d: %d is inserted\n", 
                     my_rank, which_task, data);
            else
               printf("Thread %ld: task %d: %d cannot be inserted\n", 
                     my_rank, which_task, data);
	pthread_mutex_unlock(&lock2);
	    break;
         case 1:
	pthread_mutex_lock(&lock2);
	    if (Delete(data))
               printf("Thread %ld: task %d:  %d is deleted\n", 
                     my_rank, which_task, data);
            else
               printf("Thread %ld: task %d:  %d cannot be deleted\n", 
                     my_rank, which_task, data);
	pthread_mutex_unlock(&lock2);
	    break;
         case 2:
	pthread_mutex_lock(&lock2);
	    if (Search(data))
               printf("Thread %ld: task %d:  %d is in the list\n", 
                     my_rank, which_task, data);
            else
               printf("Thread %ld: task %d:  %d is not in the list\n", 
                     my_rank, which_task, data);
	pthread_mutex_unlock(&lock2);
	    break;
         default:
            sprintf(title, "Th %ld:  task %d:  print list",
                  my_rank, which_task);
            Print(title);
            break;    
   	}
flag=1;

   		
  
	}
   return NULL;
}
/*--------------------------------------------------------------------
 * Function:    Usage
 * Purpose:     Print command line for function and terminate
 * In arg:      prog_name
 */
void Usage(char* prog_name) {
   
   fprintf(stderr, "usage: %s <number of threads> <number of tasks>\n", 
         prog_name);
   exit(0);
}  /* Usage */


/*-----------------------------------------------------------------*/
/* Insert value in correct numerical location into list */
/* If value is not in list, return 1, else return 0 */
int Insert(int value) {
   struct list_node_s* curr = head;
   struct list_node_s* pred = NULL;
   struct list_node_s* temp;
   int rv = 1;
   
   while (curr != NULL && curr->data < value) {
      pred = curr;
      curr = curr->next;
   }
   
   if (curr == NULL || curr->data > value) {
      temp = malloc(sizeof(struct list_node_s));
      temp->data = value;
      temp->next = curr;
      if (pred == NULL)
         head = temp;
      else
         pred->next = temp;
   } else { /* value in list */
      rv = 0;
   }
   
   return rv;
}  /* Insert */

/*-----------------------------------------------------------------*/
void Print(char title[]) {
   struct list_node_s* temp;
   
   printf("%s:\n   ", title);
   
   temp = head;
   while (temp != (struct list_node_s*) NULL) {
      printf("%d ", temp->data);
      temp = temp->next;
   }
   printf("\n");
}  /* Print */


/*-----------------------------------------------------------------*/
int  Search(int value) {
   struct list_node_s* temp;
   
   temp = head;
   while (temp != NULL && temp->data < value)
      temp = temp->next;
   
   if (temp == NULL || temp->data > value) {
#     ifdef DEBUG
      printf("%d is not in the list\n", value);
#     endif
      return 0;
   } else {
#     ifdef DEBUG
      printf("%d is in the list\n", value);
#     endif
      return 1;
   }
}  /* Search */

/*-----------------------------------------------------------------*/
/* Deletes value from list */
/* If value is in list, return 1, else return 0 */
int Delete(int value) {
   struct list_node_s* curr = head;
   struct list_node_s* pred = NULL;
   int rv = 1;
   
   /* Find value */
   while (curr != NULL && curr->data < value) {
      pred = curr;
      curr = curr->next;
   }
   
   if (curr != NULL && curr->data == value) {
      if (pred == NULL) { /* first element in list */
         head = curr->next;
#        ifdef DEBUG
         printf("Freeing %d\n", value);
#        endif
         free(curr);
      } else { 
         pred->next = curr->next;
#        ifdef DEBUG
         printf("Freeing %d\n", value);
#        endif
         free(curr);
      }
   } else { /* Not in list */
      rv = 0;
   }
   
   return rv;
}  /* Delete */

/*-----------------------------------------------------------------*/
void Free_list(void) {
   struct list_node_s* current;
   struct list_node_s* following;
   
   if (Is_empty()) return;
   current = head; 
   following = current->next;
   while (following != NULL) {
#     ifdef DEBUG
      printf("Freeing %d\n", current->data);
#     endif
      free(current);
      current = following;
      following = current->next;
   }
#  ifdef DEBUG
   printf("Freeing %d\n", current->data);
#  endif
   free(current);
}  /* Free_list */

/*-----------------------------------------------------------------*/
int  Is_empty(void) {
   if (head == NULL)
      return 1;
   else
      return 0;
}  /* Is_empty */
