#include <cstdlib>
#include <iostream>
#include <cstdlib>

using namespace std;

int main(int argc, char *argv[]) {
   int code;

   if (argc != 2) {
      cout << "Incorrect number of argments to exitcode" << endl;
      return(-1000000);
   }

   code = atoi(argv[1]);
   return code;

}
