#include "ConvertorASCII.h"

int main()
{
    setlocale(LC_ALL, "Ru");
    ConvertorASCII convertor_ascii;
    convertor_ascii.asking();

    cin.ignore();
    cin.get();
    return 0;
}