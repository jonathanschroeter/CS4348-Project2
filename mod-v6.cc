#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include<sstream>
#include <stdio.h>
#include <string.h>
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

//getting the "." and the ".." inode
typedef struct {
	unsigned int inode;
	unsigned char filename[28];
} dir_type;//32 Bytes long

dir_type rootpar;



int BLOCK_SIZE = 2048;
unsigned short allocate = 10000;
unsigned short direct =   04000;

int initfsFun(int fd, int n1, int n2);
int rootcreate(int fd, int freeblocks);
void AddFree();


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
		else if(strcmp(temp, "open")==0){
			cout << "temp is " << temp << endl;
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
		else if(strcmp(c2,"initfs") == 0){
			if(file == '\0'){
				cout << "You have not run open to open a file. Try Again" << endl;
			}else{
				cout << "command is " << c3 << endl;
				cout << "initializing the v6 file system. The file name is " <<
				file_name << endl;
				n1t = strtok(NULL, " ");
				n2t = strtok(NULL, " ");
				cout << "N1 and N2 are " << n1t << " and " << n2t << endl;
				n1 = atoi(n1t);
				n2 = atoi(n2t);
				initfsFun(fd,n1,n2);
			}
		}
		else{ 
			cout << "you have entered an invalid command. Try again" << endl;
		}
	}
	return 0;
}
int initfsFun(int fd, int n1, int n2){

	//opening the file
	//int fd = open(file_name.c_str(),2);
	
	superblock_type * block = new superblock_type();

	//we can have 32 i-nodes per block since block size is 2048 and i-nodes are 64 bytes
	sup.isize = n2; //how many blocks for i-nodes
	sup.fsize = n1; //file system size in number of blocks
	
	
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

	//for(int i = 0; i < freeb

	sup.flock = 0; //set to 0 since they are orginally chars in V6
	sup.ilock = 0;
	sup.fmod = 0; //if superblock has been changed
	sup.time = time(0); //getting the time


	lseek(fd,2048,SEEK_SET); //Don't want to write to 0th block, that is the root device
	
	write(fd,&sup,sizeof(superblock_type)); //writing in the super block


	//then we need to write the inodes, which will be in block 3
	//lseek(fd,BLOCK_SIZE*2,SEEK_SET); //*2 so we can make it the third block

	rootcreate(fd,freeroot);

	//what do we do for inodes??
	
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

	return 0;
}	

void addfree(int numblock){




}
