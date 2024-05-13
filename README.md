# You Spin Me Round Robin

The program performs the Round Robin scheduling algorithm for a given workload dictated by a file listing the processes and their respective PID, arrival and burst times, and a quantum length, and outputs the 
average waiting and response times. 

## Building

The program can be built on the CS111 VM on 64-bit Arch Linux using the command:
```shell
$ make
```

## Running

The program is to be run as follows:
./rr <file> quantum_length

<file> is replaced by the input file (in this case processes.txt), and quantum_length is an integer representing the time slice.

This is one possible example, for this processes.txt file:
4
1, 0, 7
2, 2, 4
3, 4, 1
4, 5, 4

```shell
$ ./rr processes.txt 3
Average waiting time: 7.00
Average response time: 2.75
```

The program thus performs the Round Robin scheduling algorithm for 4 processes, with arrival times of 0,2,4,5 and burst times of 7,4,1,4 respectively. The metrics are defined as such:

waiting time = end time - arrival time - burst time
response time = start time - arrival time

The scheduling is as follows:

P1 arrives at t0, starts at t0, takes 7, finishes at t15. 
waiting time = 8, response time = 0

P2 arrives at t2, starts at t3, takes 4, finishes at t14.
waiting time = 8, response time = 1

P3 arrives at t4, starts at t9, takes 1, finishes at t10.
waiting time = 5, response time = 5

P4 arrives at t5, starts at t10, takes 4, finishes at t16.
waiting time = 7, response time = 5

So the final output gives: average waiting time = 28/4 = 7, and average response time = 11/4 = 2.75. 


## Cleaning up

Binary files can be cleaned up with the following shell command:
```shell
$ make clean
```
