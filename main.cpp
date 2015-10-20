#include "internal.h"


using namespace std;

map <string, string *> globalVarMap;
map <string, string*> localVarMap;
vector<pid_t> allProcesses;
int debugLevel = 0;
string shellPID = to_string(getpid()); //$$
string lastForeground = to_string(-1); // $?
string lastBackground = to_string(-1); // $!

string PATH = "PATH";
string SHELL = "SHELL";
string PARENT = "PARENT";
string pathpath = get_current_dir_name();
string shellpath = get_current_dir_name();
string parentpath = get_current_dir_name();

vector<string> commandHistory;
void tryExternal(vector<string>&, string &, string &);

// sends signals to other processes
void toOtherProcess(int sigNum){
    if (allProcesses.size() <1){
        return;
    }
    for (auto it=allProcesses.begin(); it!=allProcesses.end(); ++it){
        int sendResult = kill(*it, sigNum);
        cout << "send signal result: "<< sendResult << endl;
    }
}
// Handles SIGQUIT
void sigQuitHandler(int x)
{
    toOtherProcess(x);
    cout <<"In SIGQUIT handler"<<endl; // after pressing CTRL+C you'll see this message
}
// Handles SIGCONT
void sigContHandler(int x)
{
    toOtherProcess(x);
    cout <<"In SIGCONT handler"<<endl; // after pressing CTRL+C you'll see this message
}
// Handles SIGTSTP
void sigTSTPHandler(int x)
{
    cout <<"In SIGTSTP handler"<<endl; // after pressing CTRL+C you'll see this message
}
// Handles SIGINT
void sigIntHandler(int x)
{
    //assuming allPID is a global vector that has all the PIDs of all processes in the foreground
    toOtherProcess(x);
    cout << "In SIGINT handler"<<endl; // after pressing CTRL+C you'll see this message
}
// Ignores sigabrt, alarm, hup, term, usr1, usr2
void ignoreSignals(){
    signal(SIGABRT, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGUSR1, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
}
// Sets up signal and handler relationships for signals handled internally
void handleSignals(){
    signal(SIGINT, sigIntHandler);
    signal(SIGQUIT, sigQuitHandler);
    signal(SIGCONT, sigContHandler);
    signal(SIGTSTP, sigTSTPHandler);
}
void redirectOutput(const char * filePath){
    int fd = open(filePath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    dup2(fd,1);
    dup2(fd,2);
    close(fd);
}
void redirectInput(const char * filePath){
    int fd = open(filePath,O_RDONLY);
    dup2(fd,0);
    close(fd);
}
void checkForRedirection(vector<string> & userInput, string & inputFile, string & outputFile){
    int i = 0;
    while (i < userInput.size()){
        if (userInput[i]=="<"){
            inputFile = userInput[i+1];
            userInput.erase(userInput.begin() + i);
            userInput.erase( userInput.begin() + i);
        }
        else if (userInput[i] == ">"){
            outputFile = userInput[i+1];
            userInput.erase(userInput.begin() + i);
            userInput.erase(userInput.begin() + i);
        }
        else {
            ++i;
        }
    }
    cout << "SIZE: " << userInput.size() << endl;
}
void checkForPipe(vector<string> userInput,vector<int>&pipeLocations){
    for (int i = 0; i < userInput.size(); ++i){
        if (userInput[i] == "|"){
            pipeLocations.push_back(i);
        }
    }
}
// Tries to match user input to an internal command and dispatches the
// corresponding function.
int dispatchCommand(vector<string> userInput) {
    
    if (userInput[0] == "exit") {
        if (userInput.size() > 1) {
            throw stoi(userInput[1]);
        }
        else {
            throw 0;
        }
    }
    else  if (userInput[0] == "show") {
        show(localVarMap,userInput);
    }
    else if (userInput[0] == "export"){
        export1(globalVarMap, userInput);
        
    }
    else if (userInput[0] == "wait"){
        wait(stoi(userInput[1]));
    }
    else if (userInput[0] == "pause"){
        pause2();
    }
    else if (userInput[0]== "unexport"){
        unexport(globalVarMap,userInput);
        
    }
    else if (userInput[0] == "set"){
        set1(localVarMap, userInput);
    }
    else if (userInput[0] == "unset"){
        unset1(localVarMap, userInput);
    }
    else if (userInput[0] == "help"){
        help();
    }
    else if (userInput[0] == "clr") {
        screenClear();
    }
    else if (userInput[0] == "environ") {
        printEnviron(globalVarMap);
    }
    else if (userInput[0] == "checkPath") {
        checkPath(pathpath);
    }
    else if (userInput[0] == "dir") {
        checkDir(pathpath.c_str());
    }
    else if(userInput[0] == "kill"){
        kill1(userInput);
    }
    else if (userInput[0] == "repeat"){
        if (userInput.size()>1){
            repeat(commandHistory,stoi(userInput[1]));
        }
        else{
            repeat(commandHistory,0);
        }
    }
    else if (userInput[0] == "history") {
        int historyNumber = stoi (userInput[1]);
        history(historyNumber,commandHistory);
    }
    else if (userInput[0] == "chdir") {
        if (userInput[1] == "..") {
            changePathParent(pathpath);
            
        }
        else {
            string filePath = "";
            for (int i = 1; i < userInput.size(); ++i) {
                filePath = filePath + userInput[i];
                if (i != userInput.size() - 1) {
                    filePath = filePath + " ";
                }
            }
            //check to see if it is a full path
            if (filePath[0] == '/') {
                pathpath = filePath;
                cout << "PATH: " << pathpath << endl;
            }
            else {
                typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
                vector<string> directories;
                boost::char_separator<char> sep("/");
                tokenizer tok(filePath, sep);
                for (tokenizer::iterator tokenIt = tok.begin(); tokenIt != tok.end(); ++tokenIt) {
                    directories.push_back(*tokenIt);
                }
                changeDir(pathpath, directories);
            }
        }
    }
}
// repeats command n from user's history
void repeat(vector<string> history, int n = 0){
    string redoCommand = history.at(history.size()-2-n);
    cout << redoCommand << endl;
    istringstream iss(redoCommand);
    vector<string> userInput;
    copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(userInput));
    if(checkInternal(userInput)){
        dispatchCommand(userInput);
    }
    else{
        string in = "";
        string out = "";
        tryExternal(userInput,in,out);
    }
}
// Assumes that file has one command per line, but can have multiple lines.
void fileF(vector<string> inputs) {
    // additional arguments are stored in shell variables
    int numArguments = inputs.size() -2;
    cout << numArguments << endl;
    for (int i = 0 ; i < numArguments; ++i ) {
        string index = "$" + to_string(i+1);
        cout << index << endl;
        localVarMap[index] = &(inputs.at(i+2));
    }
    // read inputs from file instead of stdin
    string fileName = inputs.at(1);
    ifstream commandInputFile(fileName);
    string modularInput;
    if(commandInputFile.is_open()) {
        while(getline(commandInputFile, modularInput)) {
            // puts each command into a vector
            istringstream iss(modularInput);
            vector<string> userInput;
            copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter(userInput));
            // sends it to method for parsing/execution of associated command
            dispatchCommand(userInput);
        }
        commandInputFile.close();
    }
    return ;
}


// This input line launches sish. Optional flags are
// -x, -d, -f.
void parse_input_line(int argc, char* argv[]) {
    setErrLog(false);
    string file = "";
    int indexOfFlags[3] = {-1,-1,-1};
    //Get flags
    
    for (int i = 1; i < argc; ++i){
        cout << argv[i] << endl;
        if (strcmp(argv[i], "-x") == 0){
            indexOfFlags[0] = i;
        }
        else if (strcmp(argv[i], "-d") == 0){
            if (argv[i+1] > 0) {
                setErrLog(true);
            }
            indexOfFlags[1] = i;
            ++i;
            debugLevel = std::stoi(argv[i], nullptr, 10);
        }
        else if (strcmp(argv[i], "-f") == 0){
            ++i;
            indexOfFlags[2] = i;
        }
    }
    // because -f flag has a variable number of arguments, we
    // find the location of the next flag in the input line if it
    // exists to parse out the -f command.
    int upperLimit = -1;
    if(indexOfFlags[2] != -1) { // if an -f flag was found
        if (max(indexOfFlags[0], indexOfFlags[1]) > indexOfFlags[2]) {
            upperLimit = max(indexOfFlags[0], indexOfFlags[1]);
        } else {
            upperLimit = argc;
        }
        // turn the -f command into vector of strings
        vector<string> input;
        for (int i = indexOfFlags[2]; i < upperLimit; ++i) {
            string token = argv[i];
            input.push_back(token);
        }
        
        fileF(input);
    } else {
        // Launch sish wherein cin is used to input commands
    }
}
// Tries to execute commands, presumably external, if they are not supported
// by sish internal commands
void tryExternal(vector<string> & userInput, string & inputFile, string & outputFile){
    
    int size = userInput[0].size() -1;
    bool background = false;
    if (userInput[0][size]=='!'){
        background = true;
        userInput[0] = userInput[0].substr(0,size);
        cout << "SUBSTR:"<<userInput[0] << endl;
    }
    bool foundSlash = false;
    int slashSearch = userInput[0].size()-1;
    while(slashSearch > -1 &&  foundSlash == false){
        if (userInput[0][slashSearch] == '/'){
            string testPath = userInput[0].substr(0,slashSearch);
            string testFile = userInput[0].substr(slashSearch+1,userInput[0].size()-1);
            vector<string> testVect;
            testVect.push_back("chdir");
            testVect.push_back(testPath);
            changeDir(pathpath,testVect);
            userInput[0] = testFile;
            foundSlash = true;
        }
        --slashSearch;
    }
    pid_t pid;
    if((pid = fork()) < 0){
        cout << "FORKING ERROR" << endl;
    }
    else if (pid == 0){
        //WE ARE THE CHILD
        cout << "Child PID is " << getpid() << endl;
        checkForRedirection(userInput,inputFile,outputFile);
        char** args = new char*[userInput.size()];
        for (int i = 0; i < userInput.size();++i){
            char copy[100];
            strcpy(copy,userInput[i].c_str());
            
            args[i] = copy;
        }
        //CHECK FOR IO REDIRECTION
        
        if(inputFile != ""){
            redirectInput(inputFile.c_str());
        }
        if(outputFile != ""){
            redirectOutput(outputFile.c_str());
        }
        execv(userInput[0].c_str(),args);
        cout << "EXEC FAILED" << endl;
        delete [] args;
    }
    else{
        //WE ARE THE PARENT
        
        if(background){
            cout << "Control Back to Parent" << endl;
            
            lastBackground = to_string(pid);
            cout << "Background ID: " << lastBackground << endl;
        }
        else{
            allProcesses.push_back(pid);
            int status;
            waitpid(pid,&status,0);
            cout <<"STATUS:"<< status << endl;
            lastForeground = to_string(status);
            cout << "Parent Printing after Fork" << endl;
            
        }
        
    }
}

int main(int argc, char* argv[]) {
    
    parse_input_line(argc, argv);
    ignoreSignals();
    signal(SIGINT, sigIntHandler);
    signal(SIGQUIT, sigQuitHandler);
    signal(SIGCONT, sigContHandler);
    signal(SIGTSTP, sigTSTPHandler);
    globalVarMap[PATH] = &pathpath;
    globalVarMap[SHELL] = &shellpath;
    globalVarMap[PARENT] = &parentpath;
    string shellPIDstring = "$$";
    string lastForegroundString = "$?";
    string lastBackgroundString = "$!";
    localVarMap[shellPIDstring] = &shellPID;
    localVarMap[lastForegroundString] = &lastForeground;
    localVarMap[lastBackgroundString]= &lastBackground;
    
    
    while (true) {
        string inputFile="";
        string outputFile="";
        vector<string> userInput;
        vector<int> pipeLocations;
        cout << "sish>> ";
        string response;
        getline(cin, response);
        commandHistory.push_back(response);
        string comment = "";
        
        
        
        
        //TOKENIZER
        typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
        boost::char_separator<char> sep("#");
        tokenizer tok(response, sep);
        int commentCount = -1;
        for (tokenizer::iterator tokenIt = tok.begin(); tokenIt != tok.end(); ++tokenIt) {
            ++commentCount;
            
            if (commentCount == 0) {
                response = *tokenIt;
            }
            else if (commentCount == 1) {
                comment = *tokenIt;
            }
        }
        istringstream iss(response);
        int i = 0;
        string holder;
        while (iss >> holder) {
            userInput.push_back(holder);
        }
        //CHECK FOR EXIT INTERNAL COMMAND
        if (checkInternal(userInput)) {
            try{
                dispatchCommand(userInput);
            }
            catch (int x){
                return x;
            }
        }
        //ELSE TRY EXTERNAL COMMAND
        else {
            tryExternal(userInput,inputFile,outputFile);     
            
        }
        
    }
}