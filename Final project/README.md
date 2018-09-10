# Client-server based ticketline service

Implementation of a *multi-threaded* ticketline service based on the distribution of tasks between different threads (ticket offices), handling the requests of clients trying to book several seats of a given cinema room.

#### Some of the covered / implemented concepts:
- *Linux API* functions and features
- **Multi-threaded** architecture
- Communication between different threads  (*FIFOs*)
- **Concurrency programming** in C (use of *Mutexes*)
