//
// Test program for Tiny I/O iostream-replacement library
//

#include <tinystream>

using namespace std;

class MyClass
{
private:
    int x;

public:
    MyClass() { x = 0; }
    ~MyClass() {}
};

int main()
{
    MyClass* m1 = new MyClass();        // Without the Tiny I/O, these would
    delete m1;                          // take boat-loads of memory.
    char* p = new char[1000];
    delete p;

    int i = 25;
    cout << "Dec:  " << dec << i << ", Hex: " << hex << i << endl;
    i = -1;
    cout << "Dec:  " << dec << i << ", Hex: " << hex << i << endl;
    unsigned j = (unsigned)-1;
    cout << "Uns:  " << dec << j << ", Hex: " << hex << j << endl;
    unsigned char k = (unsigned char)-1;
    cout << "Chr:  " << dec << k << ", Hex: " << hex << k << endl;
    unsigned long long l = (unsigned long long)-1;
    cout << "ULDec: " << dec << l << ", Hex: " << hex << l << endl;
    long long m = -1;
    cout << "SLDec: " << dec << m << ", Hex: " << hex << m << endl;

    unsigned age = 0;
    do {
        cout << dec << endl << "How old are you? ";
        cin >> age;
    } while (!age);
    cout << "In ten years, you'll be: " << age + 10 << endl << endl;

    char buf[80];
    cout << "Enter a string: ";
    cin >> buf;
    cout << "You entered: " << buf << endl << endl;

    int d;
    cout << "Enter a decimal: ";
    cin >> d;
    cout << "You entered: " << d << endl << endl;

    int dl;
    cout << "Enter a decimal: ";
    cin >> dl;
    cout << "You entered: " << dl << endl << endl;

    unsigned char uc;
    cout << "Enter an unsigned decimal between 0 and 255: ";
    cin >> uc;
    cout << "You entered: " << uc << endl << endl;

    unsigned u;
    cout << "Enter an unsigned decimal: ";
    cin >> u;
    cout << "You entered: " << u << endl << endl;

    unsigned long ul;
    cout << "Enter an unsigned decimal: ";
    cin >> ul;
    cout << "You entered: " << ul << endl << endl;

    cout << "Enter a hex number: ";
    cin >> hex >> d;
    cout << hex << "You entered: " << d << endl << endl;

    long long ld;
    cout << dec << "Enter a long long decimal: ";
    cin >> dec >> ld;
    cout << "You entered: " << ld << endl << endl;

    unsigned long long lu;
    cout << "Enter a long long unsigned decimal: ";
    cin >> lu;
    cout << "You entered: " << lu << endl << endl;

    cout << "Enter a long long hex number: ";
    cin >> hex >> ld;
    cout << hex << "You entered: " << ld << endl;


    cout << endl << "Bye!" << endl;
}



