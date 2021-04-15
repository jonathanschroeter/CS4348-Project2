#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <bits/stdc++.h>
using namespace std;

int read_command(string &command,string &file_name,int *n1,int *n2){
        cout << "Enter command" << endl;
	cin >> command;
	if(command.compare("q") == 0)
		return 0;
	cout << "Enter filename" << endl;
	cin >> file_name;
	if(file_name.compare("openfs") == 0)
		return 0;
        return 0;
}

int main(){
	string command, file_name;
	int n1, n2;
	int done = 0;
	int fd = 0;

	while(!done){
		read_command(command,file_name,&n1,&n2); //function read_command reads next command from keyboard
		if(command.compare("q") == 0)
			exit(0);
		else if(command.compare("openfs")==0){
			fd = open(file_name.c_str(),2);
			cout << "fd is " << fd << endl;
			
		}
		else if(command.compare("initfs")==0){
			fd = open("/dev/hda1",2);
			cout << "fd is " << fd << endl;

		}
	}
	return 0;
}
