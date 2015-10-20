#ifndef INTERNAL_H
#define INTERNAL_H
#include <iostream>
#include <map>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <boost/tokenizer.hpp>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <algorithm>

using namespace std;
 
 
void pause2();
void wait(int);
void kill1(vector<string> );
bool checkInternal(vector<string> &);
void changePathParent(string & path);
void checkDir(const char * path);
void changeDir(string &  path, vector<string> & userInput);
void printEnviron(map<string,string*>);
void echo(string comment);
void sho2w(vector<string> & userInput);
void screenClear();
void checkPath(string& PATH);
void history(int, vector<string>);
void repeat(vector<string>,int);
 
void export1 (map<string,string*>&,vector<string> inputs);
void unexport(map<string,string*>&,vector<string> inputs);
void show(map<string,string*>&,vector<string> inputs);
void set1(map<string,string*>&,vector<string> inputs);
void unset1(map<string,string*>&,vector<string> inputs);
void help();
void setErrLog(bool errLogIsOn);
#endif