#include <iostream>
using namespace std;

int main() {

   long long i;
   for (i=0;;i++) {
      if ((i % 100000000) == 0)
         cout << i << endl;
   }
}
