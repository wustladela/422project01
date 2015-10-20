#include "internal.h"
#include <signal.h>
using namespace std;
bool errLogOn = false;

// based on the presence of the -d flag when sish was booted,
// we turn error logs on/off
void setErrLog(bool errLogIsOn) {
	errLogOn = errLogIsOn;
}

// checks if an internal command was called
bool checkInternal(vector<string> & vect) {
	string command = vect[0];
	cout << " ERROR LOG : " << errLogOn << endl;
	return (command == "show" || command == "set" || command == "unset" ||
		command == "export" || command == "unexport" || command == "environ"
		|| command == "chdir" || command == "exit" || command == "wait" ||
		command == "clr" || command == "dir" || command == "echo" ||
		command == "help" || command == "pause" || command == "history" || command == "repeat"||command == "kill" );
}
// implements change directory- accepts both full paths and partial paths
void changePathParent(string & path) {
	unsigned int i = path.size() - 1;
	for (i; path[i] != '/'; --i) {
	}
	string parentPath = path.substr(0, i);
	cout << parentPath << endl;
	path = parentPath;
}
// executes internal command wait -l, wherein a parent processes waits for
// the termination of child process with pid = l, or if unspecified waits 
// for all child processes to termiante
void wait(int l) {
	int status, endID;
	if (l != -1) {
		do {
			endID = waitpid(l,&status, WNOHANG|WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	} else {
		wait(status);
		while (status > 0) {
			if (status == -1) {
				abort();
			}
			wait(status);
		}
	}
	return;
}
// implements kill -l pid command: sends signal number l to process with
// pid. If l is not specified, sends SIGTERM by default
void kill1(vector<string> inputs) {
	int signalNumber, targetPID;
	if (inputs.size() == 2) {
		// unspecified signal num- send SIGTERM by default
		signalNumber = 15;
		targetPID = atoi((inputs.at(1)).c_str()); 
	} else {
		string rawSignalNumber = (inputs.at(1)).substr(1,inputs.at(1).size()-1);
		signalNumber = atoi(rawSignalNumber.c_str());
		targetPID = atoi((inputs.at(2)).c_str()); 
	}
	kill(targetPID, signalNumber);
}
// pauses shell execution until user presses ENTER
void pause2() {
	if (cin.get() == '\n') {
		return;
	}

}
// implements checkDir command- allows user to check whether directory exists
// can be opened
void checkDir(const char * path) {
	DIR           *d;
	struct dirent *dir;
	d = opendir(path);
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			printf("%s\n", dir->d_name);
		}

		closedir(d);
	} else {
		if (errLogOn) {
			cout << "This director could not be opened. Try again." << endl;
		}
	}
}
// Allows user to move to different levels int eh file system
void changeDir(string &  path, vector<string> & userInput) {
	string newPath = path;
	bool exists = false;
	for (int i = 0; i < userInput.size(); ++i) {
		const char * cpath = newPath.c_str();
		exists = false;
		cout << userInput[i] << endl;
		string testDir = userInput[i];
		DIR *d;
		struct dirent *dir;
		d = opendir(cpath);
		if (d)
		{
			while ((dir = readdir(d)) != NULL)
			{
				if (strcmp(testDir.c_str(), dir->d_name) == 0) {
					exists = true;
				}
			}
			closedir(d);
		}
		if (!exists) {
			break;
		}
		else {
			newPath = newPath + "/" + testDir;
		}

		//check if this directory is inside current dir
		//if not, set exists to false
		//if true, add it onto path
	}
	if (exists) {
		path = newPath;
		cout << "New Path: " << newPath << endl;
	} else {
		if (errLogOn) {
			cout << "CHDIR: This path does not exist." << endl;
		}
	}
}
// prints out the last n commands executed by the user
void history(int n, vector<string> commandHistory) {
	int lineNum = 1;
	for (auto it = commandHistory.end() - 1; it != commandHistory.end() - 1 - n; --it){
		string outputTest = to_string(lineNum)+" "+*it;
		std::cout << outputTest << endl;
    	++lineNum;
	}
}

// prints the 3 environment 
void printEnviron(map<string,string*>globalVarMap) {
	for (map<string, string *>::iterator it = globalVarMap.begin(); it != globalVarMap.end(); ++it)
		std::cout << it->first << " => " << *(it->second) << '\n';
	
}
// prints out comment specified
void echo(string comment) {
	cout << comment << endl;
}
// prints out the values of the variables specified
void sho2w(vector<string> & userInput) {
	for (unsigned int i = 1; i < userInput.size(); ++i) {
		cout << userInput[i] << endl;
	}
}
// clears terminal screen
void screenClear() {
	cout << "\033[2J\033[1;1H";
}
// prints out path
void checkPath(string& PATH) {
	cout << PATH << endl;
}
// sets global var 1 = value of global var 2
void export1(map<string, string*> & globalVarMap,vector<string> inputs) {
	// puts value of var w2 into w1
	string w1 = inputs.at(1);
	string w2 = inputs.at(2);
	string * w1var = globalVarMap[w1];
	string * w2var = globalVarMap[w2];
	if (w1var != 0 & w2var != 0) {
		string w2value = *w2var;
		*w1var = w2value;
	} else {
		if (errLogOn) {
			cout << "These variables do not exist. Try again." <<endl;
		}
	} 
}
// deletes global variable
void unexport(map<string, string*> & globalVarMap, vector<string> inputs) {
	string w = inputs.at(1);
	map<string, string *>::iterator it;
	it = globalVarMap.find(w);
	if (it != globalVarMap.end()) {
		globalVarMap.erase(it);
	} else {
		if (errLogOn) {
			cout << "This variable does not exist. Try again." << endl;
		}
	}
}
// prints out the value of local variables specified
void show(map<string,string*> & localVarMap,  vector<string> inputs) {
	map<string, string *>::iterator it;
	for (int i = 1; i < inputs.size(); ++i) {
	  string w1 = inputs.at(i);
	  if (w1.at(0) == '$') {
	    it = localVarMap.find(w1);
	    if (it != localVarMap.end()) {
	      string * value = it-> second;
	      std::cout << (it-> first) << ": " << (*(it-> second)) << std::endl;
	    }
	  } else {
		  	if (errLogOn) {
		  		std::cout << w1 << " is not a local variable. Try again."<< std::endl;
		  	}
   		}
	}	
	return;
}
// sets local var 1 = value of local var 2
 void set1(map<string, string*> & localVarMap,vector<string> inputs) {
	string w1 = inputs.at(1);
	string w2 = inputs.at(2);
	string * w1var = localVarMap[w1];
	string * w2var = localVarMap[w2];
	if (w1var != 0 & w2var != 0) {
		string w2value = *w2var;
		*w1var = w2value;
	}
	if (errLogOn) {
		cout << "SET:: VAR 1: " << *w1var << " VAR 2: " << *w2var << endl;
	}
	return;
}
// erases local var specified
void unset1(map<string, string*> & localVarMap, vector<string> inputs) {
	string w = inputs.at(1);
	map<string, string *>::iterator it;
	it = localVarMap.find(w);
	if (it != localVarMap.end()) {
		localVarMap.erase(it);
	} else {
		if (errLogOn) {
			cout << "UNSET: Variable does not exist. Try again." << endl;
		}
	}
	return;
}
// prints out sish manual. implements more filter.
void help() {
	string line, moreInput = " ";
	ifstream myfile ("readme");
	if (myfile.is_open())
	{
		int lineCount = 0;
		for (int i = 0 ; i < 10 ; ++i) {
			if (getline(myfile, line)) {
				cout << line << endl;
			}
		}
		cout << "TYPE more TO SEE ADDITIONAL TEXT. TYPE quit TO RETURN TO SHELL" << endl;
		while ((moreInput.compare("quit") != 0))
		{
			(cin >> moreInput);
			if (moreInput.compare("more")== 0) {
				cin.clear();
				if (getline(myfile, line) ) {

				cout << line << endl;
				} else {
					return;
				}
			}
		}
		myfile.close();
		return;
	}
	else  {
		if (errLogOn) {
			cout << "Unable to open file";
		} 
	}
	return;
}
