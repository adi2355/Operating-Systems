Project Overview
This repository contains a comprehensive series of projects focused on enhancing and extending the xv6 educational operating system. XV6 (or xv6) is a reimplementation of Dennis Ritchie and Ken Thompson's Unix Version 6 (v6), developed at MIT for teaching operating system concepts. It provides a simple yet realistic implementation of a Unix-like system that can run on modern hardware while maintaining the essential architectural elements of Unix.
These projects systematically modify and extend xv6's functionality across multiple core subsystems. The work begins with basic system call implementations and command modifications, progresses through more complex process scheduling algorithms, explores virtual memory management with lazy allocation techniques, and culminates in extending the file system with support for larger files and symbolic links. Each project builds upon the previous ones, creating a progressively more capable and sophisticated operating system.
The xv6 environment provides an ideal learning platform because it includes essential operating system components (boot loader, kernel, basic shell) while remaining simple enough to be understood in its entirety. These projects leverage this foundation to explore advanced OS concepts through hands-on implementation.
Project Goals
The primary objectives of these projects are to:

Understand the internals of a Unix-like operating system: Gain deep familiarity with the architecture and implementation details of process management, memory allocation, device management, file systems, and the relationships between these subsystems.
Gain hands-on experience with system call implementation: Learn the complete lifecycle of system calls from user space to kernel and back, including argument passing, privilege transitions, error handling, and the intricacies of the C calling convention on x86.
Explore process scheduling algorithms and their implementation: Implement and compare multiple scheduling algorithms (default FIFO, SJF, lottery, priority) and understand their performance characteristics, fairness attributes, and implementation complexity.
Implement virtual memory concepts including lazy allocation: Understand page tables, address translation, trap handling, and advanced memory management techniques like on-demand (lazy) allocation with locality awareness.
Extend file system capabilities with advanced features: Modify the existing file system to support larger files through multi-level indirection, implement symbolic links, and add file seek capabilities while maintaining backward compatibility.
Develop practical skills in operating system development: Gain experience with low-level debugging techniques, system-level programming practices, and understanding complex interactions between hardware and software.
Learn to debug and test complex system-level code: Master techniques for testing and validating system-level modifications, including comprehensive approaches for verifying correctness across different execution paths.

Implementation Details
Project 1: Basic System Calls and Commands
This project focuses on fundamental operations in xv6, introducing modifications to existing commands and implementing new system calls:

Data Structures:

Process control block (struct proc in proc.h) which maintains process state, process ID, and kernel stack
File metadata structures (struct stat in stat.h) used by the ls command to distinguish between files and directories
Syscall function pointer array (syscalls[] in syscall.c) that maps system call numbers to their implementations
System call argument handling structures for parameter passing from user space to kernel


Algorithms:

Command-line argument parsing for the sleep command using the argv array passed to main
File and directory traversal for the ls command using the struct dirent directory entry structure
System call registration and dispatch mechanism that handles the transition from user space to kernel space
Process sleep/wakeup mechanism used by the sleep command to pause execution for a specified duration


Challenges and Fixes:

Understanding the complete system call implementation flow in xv6, which requires modifications to multiple files (syscall.h, syscall.c, user.h, usys.S)
Properly handling hidden files (those starting with '.') in the ls command while maintaining the original functionality
Implementing the directory indicator ('/') for directory entries in ls output
Ensuring proper error handling in the sleep command when no argument is provided
Correctly modifying the Makefile to include new user programs in the build process



Syscalls and Schedulers
This project extends syscall functionality and implements sophisticated scheduling algorithms:

Data Structures:

Process control block extensions to track scheduling-specific information including:

Predicted job length for SJF scheduling
Ticket count for lottery scheduling
Priority level for multi-level scheduling
Runtime statistics for performance analysis


Scheduling queues for maintaining separate lists of processes at different priority levels
Scheduler-specific context structures to maintain algorithm state across context switches


Algorithms:

Shortest Job First (SJF) scheduling with job length prediction mechanisms
Optional lottery scheduling with:

Ticket allocation and management
Pseudo-random number generation for fair selection
Ticket inheritance during process creation


Optional multi-level priority scheduling with:

Multiple priority queues
Priority boosting to prevent starvation
Round-robin scheduling within each priority level


uniq command implementation with support for:

Basic adjacent line deduplication
Line counting with -c flag
Unique line filtering with -u flag
Character-limited comparison with -w flag


find command implementation with comprehensive traversal and filtering based on:

Filename matching
File type filtering
Inode number comparison
Custom output formatting




Challenges and Fixes:

Modifying the timer interrupt handling system to incorporate new schedulers without disrupting existing functionality
Ensuring proper process execution time tracking across multiple scheduling decisions and context switches
Implementing flexible scheduling algorithm selection at compile time through Makefile modifications
Managing scheduler state during process creation and termination
Handling edge cases in the uniq and find commands, particularly for complex flag combinations and piped input/output



Virtual Memory
This project explores memory management in xv6, modifying the default eager allocation strategy:

Data Structures:

Page tables (multi-level data structures mapping virtual addresses to physical addresses)
Process memory size tracking in the process structure (proc->sz)
Page fault information extracted from trap frames during exception handling
Memory allocation metadata for tracking allocated pages and their attributes


Algorithms:

Trap handling for page faults, which identifies invalid memory accesses and distinguishes them from other types of exceptions
Modified sbrk() implementation that adjusts the process memory size without immediate physical memory allocation
Lazy page allocation that:

Intercepts page faults
Validates that the faulting address is within the process's virtual address space
Allocates physical memory only when accessed
Updates page tables to map the newly allocated memory
Resumes program execution transparently


Locality-aware memory allocation that:

Allocates the faulting page plus additional adjacent pages
Provides a performance improvement by reducing the number of page faults
Balances memory usage with application performance




Challenges and Fixes:

Understanding the intricate relationship between sbrk(), growproc(), and physical memory allocation in the original xv6 implementation
Identifying the correct location in the trap handling code to insert page fault handling logic
Ensuring trap handling correctly distinguishes legitimate page faults from invalid memory accesses
Managing the complexity of multiple allocation strategies selected at compile time
Correctly updating page tables to maintain memory protection and prevent unauthorized access
Handling edge cases where page faults occur at process boundaries or in stack growth scenarios



File Systems
This project significantly extends the xv6 file system capabilities:

Data Structures:

Inode structure modifications (struct dinode in fs.h) to support double-indirect blocks
File descriptor table entries that track file offsets for lseek functionality
Symbolic link metadata structure to store target path information
Block allocation tables and buffer cache structures for managing disk blocks


Algorithms:

File seeking and offset management with bounds checking and hole filling:

Repositioning file offsets arbitrarily within a file
Handling seeks beyond file end that require zero-filling


Block mapping algorithm (bmap()) enhancement to support:

Direct blocks (first 10-11 blocks)
Single-indirect blocks (next 128 blocks)
Double-indirect blocks (next 128*128 blocks)
Lazy block allocation during writes


Symbolic link traversal with:

Recursive resolution of links to targets
Cycle detection to prevent infinite loops
Support for the O_NOFOLLOW flag


Block allocation and freeing for large files with proper cleanup of all indirect blocks


Challenges and Fixes:

Maintaining backward compatibility while extending inode capabilities without increasing the inode size
Properly balancing the number of direct, indirect, and double-indirect blocks for optimal file system performance
Correctly implementing the block allocation and deallocation logic to prevent memory leaks
Handling symbolic link cycles and implementing a recursion depth limit
Ensuring file system operations (link, unlink) work correctly with symbolic links
Implementing zero-filling for sparse files when lseek creates holes



Key Architectural Decisions

System Call Implementation Approach:

New system calls are integrated through coordinated modifications to multiple files:

syscall.h: Defines system call numbers
syscall.c: Maps system call numbers to handler functions
user.h: Declares user-space function prototypes
usys.S: Contains assembly code for trap to kernel


User-facing functionality is clearly separated from kernel implementation, with user programs calling system calls through a well-defined interface
System call argument passing and validation follow a consistent pattern for safety and security
Error handling is standardized, with negative return values indicating errors and specific error codes provided where appropriate


Scheduler Design:

Modular scheduler implementation with clean separation between scheduling policy and mechanism
Compile-time scheduler selection through Makefile flags (SCHEDULER=DEFAULT, SJF, LOTTERY, or PRIORITY)
Schedulers maintain backward compatibility with existing process management functions like sleep/wakeup and process creation/termination
Process statistics tracking (using ticks_running()) provides a foundation for empirical performance comparison between schedulers
Careful interrupt handling and synchronization to ensure scheduler correctness during context switches
Priority inversion prevention through appropriate boosting mechanisms


Memory Management:

Clear separation between virtual memory policy (when to allocate) and mechanism (how to allocate)
Lazy allocation implementation that transparently catches page faults and allocates memory on demand
Locality-aware allocator that reduces the frequency of page faults by pre-allocating adjacent pages
Compile-time selection between allocation strategies via Makefile flags (ALLOCATOR=LAZY or LOCALITY)
Careful trap handling that distinguishes legitimate page faults from illegal memory accesses
Virtual memory layout preserved while changing only the allocation timing


File System Extensions:

Backward compatible inode extensions that maintain the same inode size while supporting much larger files
Strategic reduction of direct blocks (from 12 to 10) to accommodate two double-indirect block pointers
Fine-grained block allocation that only allocates indirect and double-indirect blocks when needed
Careful handling of file metadata for symbolic links stored within the inode data blocks
Complete integration of lseek functionality with existing read/write operations
Proper bounds checking and error handling for all file operations



Functions and Algorithms


hello(): A system call that prints a message from the kernel

Registers a new system call in syscall.c, syscall.h
Implements the kernel functionality in sysproc.c
Outputs "Hello from the Kernel!" to the console
Returns 0 on success


ls modifications: Enhanced directory listing command

Filters out entries starting with '.' unless -a flag is present
Appends '/' to directory names for visual differentiation
Parses command-line arguments to detect the -a flag
Preserves all original functionality while adding new features


sleep: A command that pauses execution for a specified number of ticks

Parses the command-line argument to extract sleep duration
Validates input and prints usage information if invalid
Converts string argument to integer using atoi()
Calls the sleep() system call to pause execution
Returns 0 on successful completion





ticks_running(pid): System call that returns process running time

Takes a process ID as input
Accesses the process table to check if the process exists
Returns -1 if the process doesn't exist
Returns 0 if the process exists but hasn't been scheduled yet
Returns the accumulated running time in ticks for valid processes


uniq command: Line deduplication utility with multiple modes

Basic functionality: Removes adjacent duplicate lines
-c flag: Counts and displays the number of occurrences of each line
-u flag: Shows only unique lines (those that appear exactly once)
-w N flag: Compares only the first N characters of each line
Supports file input or reading from standard input (for pipes)
Implements proper error handling for invalid inputs or file operations


find command: File search utility with various criteria

Basic function: Recursively searches directories for files matching a name
-type f flag: Matches only regular files
-type d flag: Matches only directories
-inum N flag: Matches files with exact inode number
-inum +N flag: Matches files with inode number greater than N
-inum -N flag: Matches files with inode number less than N
-printi flag: Displays the inode number before each matching file
Supports combining flags for complex search criteria


Shortest Job First (SJF) scheduler:

Assigns predicted job lengths to newly created processes using either:

Random assignment for simplicity
Exponential average based on past behavior


Selects the runnable process with the shortest predicted running time
Updates job length predictions as processes execute
Implements sjf_job_length(pid) to retrieve job length predictions


Lottery scheduler (optional):

Assigns default tickets to all processes
Provides set_lottery_tickets(tickets) to modify ticket allocation
Implements get_lottery_tickets(pid) to retrieve ticket counts
Uses pseudo-random number generation to select the next process
Distributes CPU time proportionally to assigned tickets
Handles inheritance of tickets during process creation


Priority scheduler (optional):

Supports at least three distinct priority levels
Provides set_sched_priority(priority) to adjust process priority
Implements get_sched_priority(pid) to retrieve priority levels
Always schedules higher priority processes before lower priority ones
Uses SJF or round-robin scheduling within each priority level
Includes mechanisms to prevent starvation of low-priority processes





Modified sbrk(n): Changes process memory size without allocation

Removes the call to growproc() that normally allocates physical memory
Simply increments the process size field (proc->sz) by n
Returns the old process size as the function result
Sets the stage for lazy allocation by deferring physical memory allocation


Page fault handler: On-demand memory allocation

Intercepts page faults in the trap handling code in trap.c
Verifies the faulting address is within the process's virtual address space
Extracts the faulting virtual address from the trap frame
Allocates a new physical page using kalloc()
Zeroes the newly allocated page for security
Updates the page table to map the virtual address to the physical page
Resumes program execution at the faulting instruction


Locality-aware allocation: Performance-optimized memory allocation

Calculates the page-aligned address of the faulting memory access
Allocates the faulting page plus two subsequent pages
Updates the page table for all allocated pages
Zeroes all newly allocated pages
Intelligently handles allocation at memory region boundaries
Provides a performance improvement by reducing fault frequency




lseek(fd, offset): Repositions file offset

Takes a file descriptor and desired offset as parameters
Validates the file descriptor is open and valid
Updates the file's offset position for subsequent read/write operations
Handles seeks beyond end-of-file by extending the file size
Coordinates with filewrite to zero-fill any gaps created
Returns the new position on success or appropriate error code on failure


symlink(target, path): Symbolic link creation

Creates a new file of type T_SYMLINK at the specified path
Stores the target path in the inode's data blocks
Doesn't require that the target exists when creating the link
Returns 0 on success or -1 on failure
Properly handles path lengths and storage limitations


Enhanced open(): Symbolic link handling

Detects when a path refers to a symbolic link
Recursively follows links to their targets
Respects the O_NOFOLLOW flag to open the link itself
Implements cycle detection with a maximum recursion depth
Returns appropriate error codes for invalid links


Enhanced bmap(): Multi-level block mapping

Handles direct blocks (first 10-11 blocks)
Manages single-indirect blocks (next 128 blocks)
Supports double-indirect blocks (up to 128*128 blocks per pointer)
Allocates indirect blocks only when needed
Calculates the correct block number based on logical file position
Returns the disk block number corresponding to a logical block


Enhanced itrunc(): Complete file block freeing

Releases all allocated blocks when truncating a file
Properly handles single and double-indirect blocks
Frees indirect blocks after freeing their referenced data blocks
Prevents memory leaks by ensuring all blocks are properly freed
Sets the file size to zero and updates the inode



Specific Commands
Building and Running
Copymake clean               # Cleans previous build artifacts including filesystem image
make                     # Builds xv6 with default configuration
make qemu               # Runs xv6 in QEMU emulator with GUI
make qemu-nox           # Runs xv6 in QEMU without GUI (console-only mode)

# Scheduler selection
make qemu SCHEDULER=DEFAULT   # Use default round-robin scheduler
make qemu SCHEDULER=SJF       # Use Shortest Job First scheduler
make qemu SCHEDULER=LOTTERY   # Use lottery scheduler (if implemented)
make qemu SCHEDULER=PRIORITY  # Use priority scheduler (if implemented)

# Memory allocator selection
make qemu ALLOCATOR=LAZY      # Use basic lazy allocation
make qemu ALLOCATOR=LOCALITY  # Use locality-aware allocation with prefetching
Project 1 Commands
Copyhello                   # Run the hello program (outputs "Hello Xv6!" and "Hello from the Kernel!")
ls                      # List directory contents (modified to hide hidden files and show directory indicators)
ls -a                   # List all files including hidden ones (those starting with '.')
sleep 10                # Sleep for 10 timer ticks
sleep 100               # Sleep for 100 timer ticks (approximately 10 seconds)
sleep                   # Without arguments - should display error message
Commands
Copy# uniq command examples
uniq filename.txt       # Filter adjacent repeated lines in filename.txt
uniq -c filename.txt    # Show counts of adjacent repeated lines
uniq -u filename.txt    # Show only lines that appear once (no duplicates)
uniq -w 5 filename.txt  # Compare only first 5 characters of each line
cat filename.txt | uniq # Filter repeated lines from standard input

# find command examples
find / -name file.txt              # Find file.txt starting from root
find . -name "*.c"                 # Find all .c files in current directory
find /etc -name passwd -type f     # Find only regular files named passwd
find /home -name Documents -type d # Find only directories named Documents
find / -name file.txt -inum 42     # Find files named file.txt with inode 42
find / -name "*.c" -inum +100      # Find .c files with inode number > 100
find / -name "*.h" -inum -50       # Find .h files with inode number < 50
find / -name file.txt -printi      # Find file.txt and show inode numbers

# Scheduler testing commands
ticks_running_test          # Display running time of current and other processes
simple_scheduler_test       # Test and compare SJF scheduler behavior
advanced_scheduler_test     # Test lottery or priority scheduler behavior
sjf_job_length 1            # Display predicted job length for process with PID 1
set_lottery_tickets 50      # Set current process's lottery tickets to 50
get_lottery_tickets 2       # Get lottery tickets for process with PID 2
set_sched_priority 2        # Set current process's priority to level 2 (high)
get_sched_priority 3        # Get priority level for process with PID 3
Project 3 Commands
Copy# Virtual memory testing
vm_test                     # Test lazy allocation behavior
vm_locality_test            # Test locality-aware allocation behavior

# File system commands
lseek_test                  # Demonstrate lseek functionality
symlink_test                # Test symbolic link creation and traversal
largefiles_test             # Create and verify large files with double-indirect blocks

# Example usages
symlink /etc/passwd passwd_link     # Create symbolic link to /etc/passwd
cat passwd_link                     # Access file through the link
open passwd_link O_NOFOLLOW         # Open the link itself, not the target
Future Work
Potential Improvements

Scheduler Enhancements:

Add dynamic priority adjustments based on CPU usage patterns, implementing an aging mechanism that gradually increases priority for processes that have been waiting a long time
Implement a completely fair scheduler (CFS) similar to Linux that uses a red-black tree to track process virtual runtime
Add support for real-time scheduling constraints with deadline-based scheduling policies
Implement scheduler activations for better user-level thread scheduling
Create a hybrid scheduler that dynamically switches algorithms based on system load
Add CPU affinity support for processes in a multi-core extension of xv6


Memory Management:

Implement swapping to disk for better memory utilization when physical memory is limited
Add support for memory protection between processes with explicit shared memory regions
Implement demand paging from executable files rather than just zero-filling
Add support for memory-mapped files (mmap) for more efficient file I/O
Implement copy-on-write fork() to reduce memory usage and increase fork performance
Add support for huge pages to reduce TLB pressure for large allocations
Implement a more sophisticated page replacement algorithm like clock or LIRS


File System:

Add journaling capabilities for improved reliability after system crashes
Implement file system compression to save disk space
Add support for access control lists for more fine-grained permissions
Implement a more sophisticated block allocation strategy to reduce fragmentation
Add support for file system quotas to limit user disk usage
Implement soft updates or delayed writes for performance improvement
Add support for extended attributes to store metadata alongside files
Implement readahead and write-behind in the buffer cache for better I/O performance



Known Limitations

Scheduler Limitations:

Current scheduler implementations may not handle extreme cases efficiently, such as a mix of CPU-bound and I/O-bound processes
Limited performance metrics for scheduler evaluation make it difficult to quantitatively compare algorithms
No support for process groups or sessions for job control
SJF implementation is vulnerable to starvation for long-running processes
No support for real-time constraints or deadlines
Simple priority implementation may suffer from priority inversion problems


Memory Management:

No support for memory compression or swapping, limiting total memory usage to physical RAM
Limited protection between processes could lead to security vulnerabilities
Lazy allocation implementation may not handle all edge cases, particularly around stack growth
No support for shared memory between processes for efficient inter-process communication
Locality-aware allocation may waste memory by allocating pages that are never used
No support for limiting a process's memory consumption


File System:

File size still has practical limits even with double-indirect blocks (maximum file size of ~16MB)
No support for file system journaling, making the system vulnerable to corruption after crashes
Limited symbolic link safety measures may allow complex security exploits
No atomic rename operation for safer file updates
Inefficient directory implementation for directories with many entries
No support for hard links across directories
Limited caching strategy that doesn't consider usage patterns



The projects provide a solid foundation for understanding operating system concepts, but there are many opportunities for further enhancement to create a more robust and feature-rich system. Implementing some of these improvements would provide valuable experience with advanced operating system concepts and techniques used in production operating systems.
