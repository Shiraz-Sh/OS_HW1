#include <unistd.h>
#include <iostream>

bool is_0_open(){
    char dummy;
    return read(0, &dummy, 0) >= 0;
}

void infty(){
    while (true){}
}

int main(){
    bool res = is_0_open();

    if (res){
        std::cout << "Open" << std::endl;
    }
    else{
        std::cout << "Closed" << std::endl;
    }

    close(0);
    res = is_0_open();

    if (res){
        std::cout << "Open" << std::endl;
    }
    else{
        std::cout << "Closed" << std::endl;
    }
    infty();
    return 0;
}