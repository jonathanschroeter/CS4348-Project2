Class: CS 4348.006
Professor: Subbarayan Venkatesan

Project 2: V6 File System
Group Members: Jonny Schroeter - jas170005, Caleb Munson - ccm170000
Date: 29 April 2021

Filename mod-v6.cc
Compile using command: g++ mod-v6.cc

gcc version 4.8.5 20150623 (Red Hat 4.8.5-44) (GCC)

Description:
The program has 3 commands: openfs, initfs, and count-free

# openfs - accepts 1 argument for the file name

	Description: 	In this case, file_name is the name of the file in the native unix machine (where you are running
				your program) that represents the disk drive.
				
# initfs - accepts 2 arguments, 1 for the total number of blocks, 1 for the total number of blocks for inodes
	 it is important to note that initfs cannot run unless openfs has been called at least once
	 
	Description: where n1 is the file system size in number of blocks and n2 is the number of blocks devoted to
				the i-nodes. In this case, set all data blocks free (except for one data block for storing the
				contents of i-node number 1, representing the root, which has the two entries . and .. All i-nodes
				except i-node number 1 are (unallocated) set to free. Make sure that all free blocks are
				accessible from free[] array of the super block.
				
# count-free - no arguments

	Description: 	in this case, report the number of free data blocks and number of free i-nodes available in the
				system

The program simply initializes a file system through 'initfs' in the specified file from 'openfs', and then
you can return the number of free blocks from 'count-free'