# OBLIG2 IN2140 - sirisoll - 26/03/22
## Questions
1. How to read the superblock file from disk and load the inodes into memory.
   - load_inodes() opens the superblock file and then calls create_inode()
     to recursively make inodes. I chose to use recursion after looking at
     the tree structure of the filesystem + I discovered that the IDs in the
     superblock file are connected recursively. As such, in create_inode()
     when an inode is a directory I call create_inode() on all the children
     this inode needs (here is where the entries pointer linking happens).
     The root inode is returned and load_inodes() returns this root inode.

2. Any implementation requirements that are not met
   - No, I do not believe so. Although, I never use free_block() in my
     implementation - was I supposed to do that? Would my code be better if I
     used it?

3. Any part of the implementation that deviates from the precode.
   For example, if you are creating your own files, explain what the purpose is.
   - I have one helper function create_inode() which recursively creates
     inodes as it reads from the superblock. create_inode() returns the
     root inode.

4. Any tests that fail and what you think the cause may be.
   - No tests fail with valgrind. However, I do get five warnings when i
     make all - which I honestly didn't think were a big deal so I decided
     to ignore them. Not sure if I should have fixed these?

##Running the program
In the makefile there are several options on how you can run the various
load_fs and create_fs files. Easiest way to do it is to run "make all" command
then use the valgrind_load1, valgrind_create1 etc. to tests all 6 files
for expected output and any memory leaks.
