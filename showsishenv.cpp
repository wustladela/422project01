#include <iostream>
#include <cstring>

using namespace std;

int main(int argc, char * argv[], char *envp[]) {

   int i;
   cout << endl;
   cout << "Environment Variable Listing -----------------" << endl;
   for (i=0; envp[i] != NULL; i++) {
      if (strncmp(envp[i], "PATH", 4) == 0) {
         cout << envp[i] << endl;
      }
      if (strncmp(envp[i], "Path", 4) == 0) {
         cout << envp[i] << endl;
      }
      if (strncmp(envp[i], "path", 4) == 0) {
         cout << envp[i] << endl;
      }      
      if (strncmp(envp[i], "SHELL", 5) == 0) {
         cout << envp[i] << endl;
      }
      if (strncmp(envp[i], "Shell", 5) == 0) {
         cout << envp[i] << endl;
      }
      if (strncmp(envp[i], "shell", 5) == 0) {
         cout << envp[i] << endl;
      }
      if (strncmp(envp[i], "PARENT", 6) == 0) {
         cout << envp[i] << endl;
      }
      if (strncmp(envp[i], "Parent", 6) == 0) {
         cout << envp[i] << endl;
      }
      if (strncmp(envp[i], "parent", 6) == 0) {
         cout << envp[i] << endl;
      }
      cout << endl;
    }
    cout << "-----------------------------------------------------" << endl;
}
