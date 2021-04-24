#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include<sstream>
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
unsigned short direct =   040000;

int initfsFun(string file_name, int n1, int n2);
int rootcreate(int fd, int freeblocks);



int read_command(string &command,string &file_name,int *n1,int *n2){
        cout << "Enter command" << endl;
	cin >> command;
	if(command.compare("q") == 0)
		return 0;
	cout << "Enter filename" << endl;
	cin >> file_name;
	if(file_name.compare("openfs") == 0)
		return 0;
	if(command.compare("initfs") == 0){
		cout << "Enter n1 " << endl;
		cin >> *n1;
		cout << "Enter n2 " << endl;
		cin >> *n2;
	}
        return 0;
}

int main(){
	string command, file_name;
	int n1, n2;
	int done = 0;
	int fd = 0;
	stringstream ss;
	string temp;

	while(!done){
		read_command(command,file_name,&n1,&n2); //function read_command reads next command from keyboard
		if(command.compare("q") == 0)
			exit(0);
		
		//what is the point of this??
		else if(command.compare("openfs")==0){
			fd = open(file_name.c_str(),2);
			cout << "fd is " << fd << endl;
			
		}
		else if(command.compare("initfs")==0){
			initfsFun(file_name,n1,n2);		

		}
	}
	return 0;
}
int initfsFun(string file_name, int n1, int n2){

	//opening the file
	int fd = open(file_name.c_str(),2);
	
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
	

	int freeblocks = (n1-n2-1); //1 for the boot block

	sup.flock = 0; //set to 0 since they are orginally chars in V6
	sup.ilock = 0;
	sup.fmod = 0; //if superblock has been changed
	sup.time = time(0); //getting the time


	lseek(fd,2048,SEEK_SET); //Don't want to write to 0th block, that is the root device
	
	write(fd,&sup,sizeof(superblock_type)); //writing in the super block


	//then we need to write the inodes, which will be in block 3
	//lseek(fd,BLOCK_SIZE*2,SEEK_SET); //*2 so we can make it the third block

	rootcreate(fd,freeblocks);

	//what do we do for inodes??
}

int rootcreate(int fd,int freeblocks){
	
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
		node.modtime[i] = 0;	
	}

	freeblocks = freeblocks - 1;
 
	lseek(fd,freeblocks*BLOCK_SIZE,SEEK_SET);
	write(fd,&rootpar,32);

	rootpar.filename[0] = '.';
	rootpar.filename[1] = '.';
	write(fd,&rootpar,32);


	node.addr[0] = freeblocks; //allocated blocks



	return 0;
}	
