// OctoAttach.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

int main(int argc, char* argv[])
{
    std::cout << "OctoAttach!\n";
    if (argc <= 1)
    {
        std::cout << "Invalid argument count\n";
    }

    return 0;

}
