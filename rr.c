#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers;

  /* Additional fields here */
  u32 remaining_time;
  bool in_queue;
  u32 running_at_a_time;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */
  u32 current_time = 0;
  u32 completed_processes = 0;

  for (u32 i = 0; i < size; ++i)
  {
    data[i].remaining_time = data[i].burst_time; // initialize remaining time for all processes
    data[i].in_queue = 0; // none are in the queue in the beginning
    data[i].running_at_a_time = quantum_length;
  }
    
  while (completed_processes < size) 
  {
    for (u32 i = 0; i < size; ++i) // add arrived processes to the ready queue
    {
      if (data[i].arrival_time <= current_time && data[i].remaining_time > 0 && !data[i].in_queue) 
      {
            TAILQ_INSERT_TAIL(&list, &data[i], pointers); // add processes to ready queue
            data[i].in_queue = 1;
      }
    }
        
    if (!TAILQ_EMPTY(&list)) // if ready queue isn't empty
    {
      struct process *p = TAILQ_FIRST(&list); // get process
            
      if (p->remaining_time == p->burst_time) // if running process for the first time
      {
        total_response_time += current_time - p->arrival_time; // calculate response time and update total count
      }
      
      if (p->running_at_a_time > 0 && p->remaining_time > 0)
      {
        p->running_at_a_time --; // run for one time slice
        current_time++;
        p->remaining_time --;
      }
      else if (p->running_at_a_time == 0 || p->remaining_time == 0) // finished the time slice or finished the whole process
      {
        if (p->remaining_time > 0) // if the process didn't finish
        {
          TAILQ_REMOVE(&list, p, pointers);
          TAILQ_INSERT_TAIL(&list, p, pointers); // add process back to the end of the queue if it hasn't finished
          p->in_queue = 1;
          p->running_at_a_time = quantum_length; // update new running_at_a_time for this process for next time it runs
        } 
        else // if the process finished
        {
          total_waiting_time += current_time - p->arrival_time - p->burst_time; // calculate waiting time and update total count
          completed_processes++;
          p->in_queue = 0;
          TAILQ_REMOVE(&list, p, pointers); // remove from queue
        }
      }
            
    } else current_time++; // if ready queue is empty, move to the next time unit
  }
  
  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
