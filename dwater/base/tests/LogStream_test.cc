#include "../log_stream.h"
#include "../string_piece.h"

#include <iostream>
#include <string.h>
#include <string>


int main() {
    dwater::LogStream os;
    const dwater::LogStream::Buffer &buf = os.GetBuffer();

    char                cstr[] = {'h', '3', 'l', 'l', 'o', '\0'};


    dwater::string str = "abcedfg";

    std::string stdstr = "a;sldjf;alskdjf";

    dwater::StringPiece sp(cstr);


    for ( int i = 0; i < 4; ++i ) {
        os << cstr << '\n';
    }

    for ( int i = 0; i < 8; ++i ) {
        os << sp << '\n';
    }

    os << str << "\n";
    os << stdstr << "\n";

    int                             ival = 1;
    unsigned int                    uival = 2;
    char                            c = 'B';
    short                           s = 3;
    unsigned short                  us = 4;
    long                            l = 5;
    unsigned long                   ul = 6;
    long long                       ll = 7;
    unsigned long long              ull = 8;
    float                           f = 9.9;
    double                          d = 8.8;

    os << ival << ' ' << uival << ' ' << c << ' ' << s << ' ' << us << ' '
       << l << ' ' << ul << ' ' << ll << ' ' << ull << ' ' << f << ' ' << d;

    std::cout << buf.ToString() << std::endl;
}
