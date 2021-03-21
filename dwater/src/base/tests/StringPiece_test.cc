#include "../string_piece.h"
#include <iostream>



int main() {
    dwater::StringPiece sp1;
    char c_str[] = { 'h', 'e', 'l', 'l', 'o', '\0' };
    dwater::StringPiece sp2(c_str);
    std::cout << sp1.Size() << std::endl;
    std::cout << sp2.Size() << std::endl;
    sp2.RemovePrefix(1);
    std::cout << sp2.Size() << std::endl;
    sp2.RemoveSuffix(1);
    std::cout << sp2.Size() << std::endl;

    std::cout << sp2.Empty() << std::endl;
    std::cout << sp2.AsString() << std::endl;
    std::cout << sp2.Begin() << std::endl;
    sp2.Clear();
    std::cout << sp2.Empty() << std::endl;
}
