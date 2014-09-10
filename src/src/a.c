#include <iostream> 
using namespace std; 

class Foo1 { 
public: 
    virtual void show() { // 虛擬函式 
        cout << "Foo1's show" << endl; 
    } 
}; 

class Foo2 : public Foo1 { 
public: 
    virtual void show() { // 虛擬函式 
        cout << "Foo2's show" << endl; 
    } 
}; 

class Foo3 : public Foo1 { 
}; 
void showFooByPtr(Foo1 *foo) {
    foo->show();
}

void showFooByRef(Foo1 &foo) {
    foo.show();
}

int main() { 
    Foo1 f1; 
    Foo2 f2;
	Foo3 f3;

    // 動態繫結 
    showFooByPtr(&f1); 
    showFooByPtr(&f2);
    showFooByPtr(&f3);
    cout << endl;
 
    // 動態繫結 
    showFooByRef(f1); 
    showFooByRef(f2);
    showFooByRef(f3);
    cout << endl; 

    // 靜態繫結 
    f1.show(); 
    f2.show(); 
    f3.show(); 

    return 0;
}
