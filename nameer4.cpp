#include <iostream>
#include<Windows.h>
#include "Lexical.h" 
#include "Parser.h"

using namespace std;

int main()
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    Lexical lexer; 
    if (lexer.LexicalProcess())
    {
        Parser parse;
        parse.ParserCore();
        cout << "Parsing Complete so check output files" << endl;
    }
    else {
        cout << "Failed!" << endl;
    }

    return 0;
}