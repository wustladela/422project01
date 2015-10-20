#include <iostream>

using namespace std;

int main(int argc, char * argv[], char *envp[]) {

   int i;
   cout << endl;
   cout << "Environment Variable Listing -----------------" << endl;
   for (i=0; envp[i] != NULL; i++) {
      cout << envp[i] << endl;
   }
   cout << "----------------------------------------------" << endl;
}
