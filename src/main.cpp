#include "ConvertorASCII.h"

int main()
{
    setlocale(LC_ALL, "Ru");
    ConvertorASCII convertor_ascii;
    convertor_ascii.asking();

    std::cin.ignore();
    std::cin.get();
    return 0;
}