/*
Team Members: Jonathan Schroeter (Jas170005@utdallas.edu) and Caleb Munson (ccm170000@utdallas.edu)

Work: 	Program was worked on at the same time by both members for all parts. No work was divided.
	Code Comments: Jonathan Schroeter
	README: Caleb Munson	

Date: 4/29/2021

Class: CS/SE 4348.006

How the Program is Run: The program is compiled by using g++ mod-v6.cc, then ran by running ./a.out

MAJOR DECISIONS MADE:
	-If free[] and nfree are stored in a data block, that block is no longer free
		-At those data blocks, only nfree and free[] are stored
	-After "initfs" has been run, "openfs" must be run again in order for "initfs" to be run
	-count-free can only be run after "openfs" and "initfs" have been run
*/

//Headers
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include<sstream>
#include <stdio.h>
#include <string.h>
#include <bits/stdc++.h>
using namespace std;


//superblock struct
typedef struct{
	int isize;
	int fsize;
	int nfree;
	unsigned int free[250];
	unsigned int ninode;
	unsigned int inode[250];
	int flock;
	int ilock;
	unsigned int fmod;
	unsigned int time;
} superblock_type;

//Main superblock 
superblock_type sup;

//temp superblock used for count-free
superblock_type storer;

//inode struct
typedef struct {
	unsigned short flags;
	unsigned int nlinks;
	unsigned int uid;
	unsigned int gid;
	unsigned int size;
	unsigned int addr[9];
	unsigned short actime[2];
	unsigned short modtime[2];
	unsigned short dummy; //not used
} inode_type; //64 Bytes in size

//inode type
inode_type node;

//for the "." and the ".."
typedef struct {
	unsigned int inode;
	unsigned char filename[28];
} dir_type;//32 Bytes long


//for the root data block
dir_type rootpar;


//global variable
int BLOCK_SIZE = 2048; //block size
vector<int> position; //vector used for count free
int BLOCK_BOOT = 0; //used in count free

//extra 0 added at the front since it is a short
unsigned short allocate = 0100000; //if it is allocated
unsigned short unallocate = 0000000; //if it is unallocated
unsigned short direct =   0040000; //directory file
unsigned short owner = 0000764; //rwx- owner rw - group r - everyone else

//functions
int initfsFun(int fd, int n1, int n2);
int rootcreate(int fd, int freeblocks);
void addfree(int i, int fd,int bootInode);
void countfree(string filename);
void inodecreate(int fd, int block);

//main function
int main(){

	//variables
	char command[300];
	char c2[300];
	char c3[300];
	string file_name;
	int n1, n2;
	char *file = '\0';
	char *n1t = '\0';
        char *n2t = '\0';
	char *dummy = '\0';
	int done = 0;
	bool init = false;
	int fd = 0;
	stringstream ss;
	string temp;
	int opencount = 0;


	//loop for getting user commands
	while(!done){

		//getting the user commands
		cout << "Enter command" << endl;
        	scanf(" %[^\n]s", command);
		strncpy(c2,command,300);
		strncpy(c3,command,300);
		char *temp;
                temp = strtok(c2, " ");

		//if the command = q
		if(strcmp(command,"q") == 0){

			//exit the program
			cout << "The program will now exit" << endl;
			exit(0);
		}

		//if command is for openfs
		else if(strcmp(temp, "openfs")==0){
		
			//getting file name
			file = strtok(NULL, " ");

			//emptying the file
			std::ofstream ofs;
			ofs.open(file, std::ofstream::out | std::ofstream::trunc);
			ofs.close();
			
			//opening the file
			fd = open(file,2);

			//resetting the vectory and block_boot
			while(position.size() != 0){
				position.pop_back();
			}
			BLOCK_BOOT = 0;

			//if the file is ""
			if(fd == -1){
				cout << "The file does not exist. Try again" << endl;
			}
			else{

				//opening the file
				cout << "Opening file " << file << endl;
				file_name = file;
				opencount = 0;
			}
		}

		//command is initfs
		else if(strcmp(temp,"initfs") == 0){
			
			//if openfs has not been run first
			if(file == '\0'){
				cout << "You have not run open to open a file. Try Again" << endl;
			
			//if initfs has already been run, openfs must be run to inti it again
			}else if(opencount != 0){
				cout << "You must run openfs again to open the file inorder to intialize it again " << endl;	
			}else{
			
				cout << "command is " << temp << endl;
				cout << "initializing the v6 file system. The file name is " <<
				file_name << endl;
			
				//getting n1 and n2
				n1t = strtok(NULL, " ");
				n2t = strtok(NULL, " ");
				if(n1t == NULL || n2t == NULL){
					cout << "You have entered invalid sizes for n1 or n2. Try again" << endl;
				}else{

					//changing n2 and n2 into numbers
					cout << "N1 and N2 are " << n1t << " and " << n2t << endl;
					n1 = atoi(n1t);
					n2 = atoi(n2t);

					//checking correct sizes for n1 and n2
					if(n1 < 4 || n2 > n1 || (n1-n2) <= 2){
						cout << "You have entered invalid sizes for n1 or n2. Try again" << endl;
					}else{

						//initializing the file
						initfsFun(fd,n1,n2);
						init = true;
						opencount = 1;
					}
				}
			}
		}

		//if command is count-free
		else if(strcmp(temp,"count-free") == 0){
			if(file == '\0' || init == false || opencount == 0)
				cout << "The file has not been opened or not been initalized. Try again." << endl;
			else

				//counting the free inodes and data blocks
				countfree(file_name);
			
		}
		else{ 
			cout << "you have entered an invalid command. Try again" << endl;
		}
	}
	return 0;
}

//initializing the file system
int initfsFun(int fd, int n1, int n2){


	//we can have 32 i-nodes per block since block size is 2048 and i-nodes are 64 bytes
	sup.isize = n2; //how many blocks for i-nodes
	sup.fsize = n1; //file system size in number of blocks
	int bootInode = 2 + n2; 
	
	//filling up free with dummy data
	for(int i = 0; i < 250; i++){
		sup.free[i] = 0; 		
	}

	//setting nfree to 0
	sup.nfree = 0; 


	sup.ninode = n2 * 32; //number of free inodes
	
	//first block of data blocks
	BLOCK_BOOT = (n2 + 2) * BLOCK_SIZE;

	

	//filling up with information
	if(sup.ninode > 249){
		for(int i = 1; i < 249; i++){
                	sup.inode[i] = i;
        	}
	}else{
		for(int i = 1; i < sup.ninode; i++){
			sup.inode[i] = i;
		}
	}
	

	int freeroot = 2 + n2; //block for the root
	int numdblock = (n1 - n2 - 2); //number of free data blocks

	sup.flock = 0; //set to 0 since they are orginally chars in V6
	sup.ilock = 0; //dummy
	sup.fmod = time(0); //if superblock has been changed
	sup.time = time(0); //getting the time

	
	lseek(fd,2048,SEEK_SET); //Don't want to write to 0th block, that is the root device
	
	write(fd,&sup,sizeof(superblock_type)); //writing in the super block

	//adding free datablocks
	for(int i = 0; i < numdblock; i++){
		addfree(i,fd,bootInode);
	}

	//creating the root
	rootcreate(fd,freeroot);

	//creating the rest of the inodes
	for(int i = 1; i < sup.ninode; i++){
		inodecreate(fd,i);
	}
	
	close(fd);
}

int rootcreate(int fd,int freeroot){
	
	//creating for "." and ".."		
	rootpar.filename[0] = '.';
	rootpar.inode = 1;

	//flags for the inode
	node.flags = allocate | direct | owner;
	node.nlinks = 1;
	node.uid = 0;
	node.gid = 0;
	node.size = 0;

	//dummy data
	for(int i = 0; i < 9; i++){
		node.addr[i] = 0;
	}

	for(int i = 0; i < 2;i++){
		node.actime[i] = 0;	
	}
	
	for(int i = 0; i < 2;i++){
		node.modtime[i] = time(0);	
	}


	//get to the rootfree block
	lseek(fd,freeroot*BLOCK_SIZE,SEEK_SET);
	
	//writing to root data block
	write(fd,&rootpar,32);
	rootpar.filename[0] = '.';
	rootpar.filename[1] = '.';
	write(fd,&rootpar,32);

	node.addr[0] = freeroot; //set addr[0] to the block that "." ".."
	
	//writing to first inode
	lseek(fd,2*BLOCK_SIZE,SEEK_SET);
	write(fd,&node,sizeof(inode_type));
	
	//updating the superblock
	sup.ninode = sup.ninode - 1;
	sup.nfree--;
	sup.fmod = time(0);
	
	//writing to the superblock
	lseek(fd,BLOCK_SIZE,SEEK_SET);
	write(fd,&sup,sizeof(superblock_type));

	return 0;
}	

//counting the number of free data blocks and inodes
void countfree(string filename){

	//variables
	int ntemp;
	int allocated = position.size();
	int fd = open(filename.c_str(),2);
	vector<int> vectemp(position);
        unsigned int temper[250];
	int acc = sup.nfree;

	//getting the number of free inodes
	cout << "The number of free i-nodes for the V6 file system " << filename << " is " << sup.ninode << endl;

	//if free[] does not hold references to a data block where another free[] will be stored
	if(sup.free[0] == 0){
		cout << "The number of free data blocks for the v6 file system " << filename << " is " << sup.nfree << endl;
	
	//if free[] does references a datablock
	}else{
		
		//getting the ammount of free datablocks
		int block = sup.free[0];
		while(vectemp.size() != 0){

			//used for telling how many times the loop needs to run
			int temp = vectemp.back();
			vectemp.pop_back();

			//finding where in the file free[] is stored
			int loc = BLOCK_BOOT + (block * BLOCK_SIZE);
			lseek(fd,loc,SEEK_SET);

			//getting nfree and free[] out of that datablock
			read(fd,&storer.nfree,sizeof(sup.nfree));
			read(fd,&storer.free,sizeof(sup.free));
                        //read(fd,&storer,sizeof(superblock_type));
			//cout << "nfree from the file is " << storer.nfree << endl;

			//adding to the accumulator for the ammount of free data blocks 
			acc = acc + storer.nfree;
			
			//getting the next possible datablock where nfree and free[] could be stored
			block = storer.free[0];
		}

		//outputting the number of free data blocks
		cout << "The number of free data blocks for the v6 file system " << filename << " is " << acc << endl;
		
	}
	close(fd);
}

//adding a free data block to the v6 file
void addfree(int numblock, int fd,int bootInode){


	//locating where in the file the data block should go
	int blockArea = (bootInode*BLOCK_SIZE) + (numblock * BLOCK_SIZE);
	BLOCK_BOOT = (bootInode * BLOCK_SIZE);

	//if there's less than 250 in nfree
	if(sup.nfree < 250){
		sup.free[sup.nfree] = numblock;
		sup.nfree++;
	
	//if there is more than 250, nfree and free[] need to be stored at the current data block
	}else{
		position.push_back(0);

		//locating where the current data block is in the file
		lseek(fd,blockArea,SEEK_SET);

		//decrementing nfree since storing at the datablock will make the datablock no longer free
		sup.nfree--;

		//writing to the datablock
		write(fd,&sup.nfree,sizeof(sup.nfree));
		write(fd,&sup.free,sizeof(sup.free));
		//write(fd,&sup,sizeof(superblock_type));
		
		//resetting free[]
		for(int i = 1; i < 249;i++){
			sup.free[i] = 0;
		}

		//setting free[0] to where nfree and free[] where just stored
		sup.free[0] = numblock;

		//setting nfree to 1 now
		sup.nfree = 1;

		//updating the time for the superblock
		sup.fmod = time(0);
	}
	lseek(fd,BLOCK_SIZE,SEEK_SET);
	write(fd,&sup,sizeof(superblock_type));
}

//creating inodes
void inodecreate(int fd, int block){
	
	//for figuring out where in the file to write
	int next = block * 64;

	//data for the unallocated inodes
	node.flags = unallocate;
        node.nlinks = 0;
        node.uid = 0;
        node.gid = 0;
        node.size = 0;


	//dummy data for the inodes
        for(int i = 0; i < 9; i++){
                node.addr[i] = 0;
        }

        for(int i = 0; i < 2;i++){
                node.actime[i] = time(0);
        }

        for(int i = 0; i < 2;i++){
                node.modtime[i] = time(0);
        }

	//writing the inodes to the file
	lseek(fd,((2*BLOCK_SIZE)+next),SEEK_SET);
        write(fd,&node,sizeof(inode_type));
}
