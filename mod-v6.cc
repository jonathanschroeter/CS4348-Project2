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

superblock_type sup;

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

inode_type node;

//for the "." and the ".."
typedef struct {
	unsigned int inode;
	unsigned char filename[28];
} dir_type;//32 Bytes long

dir_type rootpar;



int BLOCK_SIZE = 2048;
vector<int> position;
int BLOCK_BOOT = 0;
unsigned short allocate = 0100000;
unsigned short unallocate = 0000000;
unsigned short direct =   0040000;
int initfsFun(int fd, int n1, int n2);
int rootcreate(int fd, int freeblocks);
void addfree(int i, int fd,int bootInode);
void countfree(string filename);
void inodecreate(int fd, int block);

int main(){
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


	while(!done){
		cout << "Enter command" << endl;
        	scanf(" %[^\n]s", command);
		strncpy(c2,command,300);
		strncpy(c3,command,300);
		char *temp;
                temp = strtok(c2, " ");
		if(strcmp(command,"q") == 0){
			cout << "The program will now exit" << endl;
			exit(0);
		}
		else if(strcmp(temp, "openfs")==0){
			file = strtok(NULL, " ");
			std::ofstream ofs;
			ofs.open(file, std::ofstream::out | std::ofstream::trunc);
			ofs.close();
			fd = open(file,2);
			if(fd == -1){
				cout << "The file does not exist. Try again" << endl;
			}
			else{
				cout << "Opening file " << file << endl;
				file_name = file;
			}
		}
		else if(strcmp(temp,"initfs") == 0){
			if(file == '\0'){
				cout << "You have not run open to open a file. Try Again" << endl;
			}else{
				cout << "command is " << temp << endl;
				cout << "initializing the v6 file system. The file name is " <<
				file_name << endl;
				n1t = strtok(NULL, " ");
				n2t = strtok(NULL, " ");
				if(n1t == NULL || n2t == NULL){
					cout << "You have entered invalid sizes for n1 or n2. Try again" << endl;
				}else{
					cout << "N1 and N2 are " << n1t << " and " << n2t << endl;
					n1 = atoi(n1t);
					n2 = atoi(n2t);
					if(n1 < 4 || n2 > n1 || (n1-n2) <= 2){
						cout << "You have entered invalid sizes for n1 or n2. Try again" << endl;
					}else{
						initfsFun(fd,n1,n2);
						init = true;
					}
				}
			}
		}
		else if(strcmp(temp,"count-free") == 0){
			if(file == '\0' || init == false)
				cout << "The file has not been opened or not been initalized. Try again." << endl;
			else
				countfree(file_name);
			
		}
		else{ 
			cout << "you have entered an invalid command. Try again" << endl;
		}
	}
	return 0;
}
int initfsFun(int fd, int n1, int n2){


	//we can have 32 i-nodes per block since block size is 2048 and i-nodes are 64 bytes
	sup.isize = n2; //how many blocks for i-nodes
	sup.fsize = n1; //file system size in number of blocks
	int bootInode = 2 + n2; 
	
	//this needs to be fixed!!!!
	for(int i = 0; i < 250; i++){
		sup.free[i] = 0; //what should we fill in free with??		
	}

	sup.nfree = 0; 

	sup.ninode = n2 * 32; //number of free inodes


	
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
	sup.ilock = 0;
	sup.fmod = 0; //if superblock has been changed
	sup.time = time(0); //getting the time


	lseek(fd,2048,SEEK_SET); //Don't want to write to 0th block, that is the root device
	
	write(fd,&sup,sizeof(superblock_type)); //writing in the super block


	//then we need to write the inodes, which will be in block 3
	//lseek(fd,BLOCK_SIZE*2,SEEK_SET); //*2 so we can make it the third block

	for(int i = 0; i < numdblock; i++){
		addfree(i,fd,bootInode);
	}
	rootcreate(fd,freeroot);

	//creating the rest of the inodes
	for(int i = 1; i < sup.ninode; i++){
		inodecreate(fd,i);
	}
	
	close(fd);
}

int rootcreate(int fd,int freeroot){
	
	//creating for the parent

		
	rootpar.filename[0] = '.';

	rootpar.inode = 1;

	node.flags = allocate | direct;
	node.nlinks = 0;
	node.uid = 0;
	node.gid = 0;
	node.size = 0;

	
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
	write(fd,&rootpar,32);
	rootpar.filename[0] = '.';
	rootpar.filename[1] = '.';
	write(fd,&rootpar,32);

	node.addr[0] = freeroot; //set addr[0] to the block that "." ".."

	lseek(fd,2*BLOCK_SIZE,SEEK_SET);
	write(fd,&node,sizeof(inode_type));
	
	sup.ninode = sup.ninode - 1;
	sup.nfree = sup.nfree - 1;
	lseek(fd,BLOCK_SIZE,SEEK_SET);
	write(fd,&sup,sizeof(superblock_type));

	return 0;
}	
void countfree(string filename){

	int ntemp;
	int allocated = position.size();
	int fd = open(filename.c_str(),2);
        unsigned int temper[250];
	int acc = sup.nfree;
	cout << "The number of free i-nodes for the V6 file system " << filename << " is " << sup.ninode << endl;

	if(sup.free[0] == 0){
		cout << "The number of free data blocks for the v6 file system " << filename << " is " << sup.nfree << endl;
	}else{
		while(position.size() != 0){
			int block = sup.free[0];
			int temp = position.front();
			position.erase(position.begin());
			lseek(fd,temp,SEEK_SET);
                        read(fd,&storer,sizeof(superblock_type));
			cout << "nfree from the file is " << storer.nfree << endl; 
			acc = acc + storer.nfree;
		}
		cout << "The number of free data blocks for the v6 file system " << filename << " is " << acc-allocated << endl;
		
	}
	close(fd);
}
void addfree(int numblock, int fd,int bootInode){

	int blockArea = (bootInode*BLOCK_SIZE) + (numblock * BLOCK_SIZE);
	BLOCK_BOOT = (bootInode * BLOCK_SIZE);
	if(sup.nfree < 250){
		sup.free[sup.nfree] = numblock;
		sup.nfree++;
	}else{
		position.push_back(blockArea);
		lseek(fd,blockArea,SEEK_SET);
		sup.nfree++;
		sup.free[sup.nfree] = numblock;
		cout << "nfree is past 250, writing to block number " << numblock << endl;
		write(fd,&sup,sizeof(superblock_type));
		for(int i = 0; i < 250;i++){
			sup.free[i] = 0;
		}
		sup.free[0] = numblock;
		sup.nfree = 0;
	}
	lseek(fd,BLOCK_SIZE,SEEK_SET);
	write(fd,&sup,sizeof(superblock_type));
}
void inodecreate(int fd, int block){

	int next = block * 64;
	node.flags = unallocate;
        node.nlinks = 0;
        node.uid = 0;
        node.gid = 0;
        node.size = 0;


        for(int i = 0; i < 9; i++){
                node.addr[i] = 0;
        }

        for(int i = 0; i < 2;i++){
                node.actime[i] = 0;
        }

        for(int i = 0; i < 2;i++){
                node.modtime[i] = 0;
        }
	lseek(fd,((2*BLOCK_SIZE)+next),SEEK_SET);
        write(fd,&node,sizeof(inode_type));
}
