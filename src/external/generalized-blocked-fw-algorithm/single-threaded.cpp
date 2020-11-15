#include <iostream>
#include <vector>
#include <string>

#include "../../utilz/matrix.h"
#include "../../utilz/graphs.h"

using namespace std;

int main()
{
    vector<string> msg {"Hello", "C++", "World", "from", "VS Code", "and the C++ extension!"};
    math::utilz::square_matrix<long> a;

    for (const string& word : msg)
    {
        cout << word << " ";
    }
    cout << endl;
}
