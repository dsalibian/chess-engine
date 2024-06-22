#include <chrono>
#include <iostream>

#include "movegen.h"
#include "position.h"
#include "misc.h"

using namespace std;

int main() {


    cout << chrono::steady_clock::now().time_since_epoch().count() << endl;;

    return 0;
}
