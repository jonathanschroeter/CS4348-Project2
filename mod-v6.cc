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



//getting the "." and the ".." inode???
typedef struct {
	unsigned int inode;
	unsigned char filename[28];
} dir_type;//32 Bytes long

int initfsFun(string file_name, int n1, int n2);



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


	//we can have 32 i-nodes per block since block size is 2048 and i-nodes are 64 bytes
	sup.isize = n2; //how many blocks for i-nodes
	sup.fsize = n1; //file system size in number of blocks
	
	for(int i = 0; i < 250; i++){
		sup.free[i] = 0; //what should we fill in free with??		
	}	
	sup.ninode = n2; //should the number of free be all of them when initialzing???

	for(int i = 0; i < 250; i++){
		sup.inode[i] = 0; //should this be set to 0 when initilizing??
	}
	sup.flock = 0; //set to 0 since they are orginally chars in V6
	sup.ilock = 0;
	sup.fmod = 0; //if superblock has been changed
	sup.time = 0; //set to 0 since we don't understand how to make time an int

	//How do we write this? what do we do??

	lseek(fd,2048,SEEK_SET); //Don't want to write to 0th block, that is the root device

	write(fd,&sup,2048); //writing in the super block


	//then we need to write the inodes, which will be in block 3
	lseek(fd,2048*2,SEEK_SET); //*2 so we can make it the third block

	//what do we do for inodes??
}
