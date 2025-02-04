#ifndef LEXICAL_H
#define LEXICAL_H

#include <iostream>
#include <fstream>
#include <cctype>
#include <string>
#include <unordered_set>

#define MAX_STATES 10
#define MAX_COLUMNS 10

using namespace std;

class Lexical
{
private:
    // Global output files
    ofstream tokenFile;
    ofstream errorFile;

    // Global Counters
    int nKeywords = 0;
    int nIdentifiers = 0;
    int nNumbers = 0;
    int nPunctuations = 0;
    int nOperators = 0;
    int nInvalid = 0;

    // Keyword set (using unordered_set for faster lookup)
    unordered_set<string> keywords = {
        "loop", "agar", "magar", "asm", "else", "new", "this", "auto",
        "enum", "operator", "throw", "bool", "explicit", "private", "true",
        "break", "export", "protected", "try", "case", "extern", "public",
        "typedef", "catch", "false", "register", "typeid", "char", "float",
        "typename", "class", "for", "return", "union", "const", "friend",
        "short", "unsigned", "goto", "signed", "using", "continue", "if",
        "sizeof", "virtual", "default", "inline", "static", "void", "delete",
        "int", "volatile", "do", "long", "struct", "double", "mutable",
        "switch", "while", "namespace"
    };

    int identifierTable[5][4] = 
    {
        {    2,    -1,    1,    -1 },
        {    3,     3,    3,    -1 },
        {    2,     2,    3,    -1 },
        {    3,     3,    3,    -1 },
        {   -1,    -1,   -1,    -1 } 
    };

    int numberTable[8][6] = 
    {
        {  2,      1,       3,      -1,     -1 }, 
        {  2,     -1,       3,      -1,     -1 },
        {  2,     -1,       4,       5,     -1 },
        {  4,     -1,      -1,      -1,     -1 },
        {  4,     -1,      -1,       5,     -1 },
        {  7,      6,      -1,      -1,     -1 },
        {  7,     -1,      -1,      -1,     -1 }, 
        {  7,     -1,      -1,      -1,     -1 },
    };

    int punctuationTable[7][7] = 
    {
        {  1,   2,   3,   4,   5,   6,  -1 },
        { -2,  -2,  -2,  -2,  -2,  -2,  -2 }, 
        { -2,  -2,  -2,  -2,  -2,  -2,  -2 }, 
        { -2,  -2,  -2,  -2,  -2,  -2,  -2 },
        { -2,  -2,  -2,  -2,  -2,  -2,  -2 },
        { -2,  -2,  -2,  -2,  -2,  -2,  -2 }, 
        { -2,  -2,  -2,  -2,  -2,  -2,  -2 }  
    };

    int operatorTable[12][13] = 
    {
        {  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, -1 }, 
        { -1, -1, -1, 13, -1, -1, -1, -1, -1, -1, -1, -1, -2 }, 
        { -1, 14, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2 }, 
        { -1, -1, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2 },
        { 13, 15, 15, 16, -1, -1, 20, -1, -1, -1, -1, -1, -2 }, 
        { -1, -1, -1, 17, 18, -1, -1, -1, -1, -1, -1, -1, -2 }, 
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2 },
        { -1, -1, -1, -1, -1, -1, 21, -1, -1, -1, -1, -1, -2 },
        { -1, -1, -1, -1, -1, -1, -1, -1, 24, -1, -1, -1, -2 }, 
        { -1, -1, -1, -1, -1, -1, -1, -1, 24, -1, -1, -1, -2 }, 
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, 25, -1, -1, -2 }, 
        { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 26, -1, -2 } 
    };



public:
    bool LexicalProcess()
    {
        tokenFile.open("token.txt");
        errorFile.open("errors.txt");
        const char* inputFileName = "test_code.txt";
        ifstream inputFile(inputFileName);
        if (!inputFile.is_open())
        {
            cerr << "Error opening input file.\n";
            return false;
        }
        string line;
        int lineNum = 0;
        while (getline(inputFile, line))
        {
            lineNum++;
            string token;
            for (size_t i = 0; i < line.length(); ++i)
            {
                char c = line[i];
                if (isspace(c) || c == '$' || c == ',' || c == ';' || c == '(' || c == ')')
                {
                    if (!token.empty())
                    {
                        processToken(token, lineNum);
                        token.clear();
                    }
                    continue;
                }
                token += c;
            }
            if (!token.empty())
            {
                processToken(token, lineNum);
            }
        }
        cout << endl;
        cout << "Keywords: " << nKeywords << endl;
        cout << "Identifiers: " << nIdentifiers << endl;
        cout << "Numbers: " << nNumbers << endl;
        cout << "Punctuations: " << nKeywords << endl;
        cout << "Operators: " << nOperators << endl;
        cout << "Invalid: " << nInvalid << endl;
        cout << "Total Tokens: " << nKeywords + nIdentifiers + nNumbers + nKeywords + nOperators << endl << endl;

        // File Output to Token
        tokenFile << endl;
        tokenFile << "Keywords: " << nKeywords << endl;
        tokenFile << "Identifiers: " << nIdentifiers << endl;
        tokenFile << "Numbers: " << nNumbers << endl;
        tokenFile << "Punctuations: " << nKeywords << endl;
        tokenFile << "Operators: " << nOperators << endl;
        tokenFile << "Invalid: " << nInvalid << endl;
        tokenFile << "Total Tokens: " << nKeywords + nIdentifiers + nNumbers + nKeywords + nOperators << endl;

        // File Output to Error
        errorFile << endl;
        errorFile << "Invalid: " << nInvalid << endl;
        errorFile << "Total Tokens: " << nKeywords + nIdentifiers + nNumbers + nKeywords + nOperators << endl;

        inputFile.close();
        tokenFile.close();
        errorFile.close();
        cout << "Lexical analysis done. proceeding with Parsing\n\n\n";
        return true;
    }


    //                                  Mapping Functions 
                            
    int getIdentifierCol(char c)
    {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
            return 0;                                          // Letter
        if (c >= '0' && c <= '9')
            return 1;                                         // Digit
        if (c == '_')
            return 2;                                         // Underscore
        return 3;                                            // Other (invalid input)
    }

    int getNumberCol(char c)
    {
        if (c >= '0' && c <= '9')
            return 0; // Digit
        if (c == '+' || c == '-')
            return 1; // Sign
        if (c == '.')
            return 2; // Decimal point
        if (c == 'e' || c == 'E')
            return 3; // Exponent
        return 4;     // Other (invalid input)
    }

   
    int getPunctuationCol(char c)
    {
        switch (c)
        {
        case '[':
            return 0;
        case '{':
            return 1;
        case '<':
            return 2;
        case '>':
            return 3;
        case '}':
            return 4;
        case ']':
            return 5;
        default:
            return 6; // Other (invalid input)
        }
    }

    int getOperatorCol(char c)
    {
        switch (c)
        {
        case '!':
            return 0;
        case '<':
            return 1;
        case '>':
            return 2;
        case '=':
            return 3;
        case ':':
            return 4;
        case '*':
            return 5;
        case '+':
            return 6;
        case '/':
            return 7;
        case '-':
            return 8;
        case '&':
            return 9;
        case '|':
            return 10;
        case '%':
            return 11;
        default:
            return 12; // Invalid input
        }
    }



    //                                              FSM Functions                                                   

    int identifierFSM(const std::string& token)
    {
        int state = 0; // Start state

        for (char c : token)
        {
            int col = getIdentifierCol(c);
            state = identifierTable[state][col];

            // If invalid state is reached, return -1
            if (state == -1)
                return -1;
        }

        // Final valid state: 3 (valid identifier)
        if (state == 3)
            return state;
        return -1; // Not a valid identifier
    }

    int numberFSM(const std::string& token)
    {
        int state = 0; // Start state

        for (char c : token)
        {
            int col = getNumberCol(c);
            state = numberTable[state][col];

            // If invalid state is reached, return -1
            if (state == -1)
                return -1;
        }

        // Final valid states: 2 (integer), 4 (decimal), 7 (exponent part)
        if (state == 2 || state == 4 || state == 7)
            return state;
        return -1; // Not a valid number
    }

    int punctuationFSM(const std::string& token)
    {
        int state = 0; // Start state

        for (char c : token)
        {
            int col = getPunctuationCol(c);
            state = punctuationTable[state][col];

            // If invalid or terminal state is reached, return error
            if (state == -1 || state == -2)
                return -1;
        }

        return state; // Final state (if valid)
    }

    int operatorFSM(char currentChar, int currentState, char nextChar)
    {
        int col = getOperatorCol(currentChar);
        if (col == 12) return -1; // Invalid input

        int nextState = operatorTable[currentState][col];
        //cout << "Current Char: " << currentChar << "          NextChar: " << nextChar << "            CurrentState: " << currentState << "        Next State: "<< nextState<< endl;

        // Lookahead for multi-character operators
        if (currentState == 1 && nextChar == '=') return 13;  // !=
        if (currentState == 2 && nextChar == '>') return 14;  // <>
        if (currentState == 2 && nextChar == '<') return 14;  // <<
        if (currentState == 3 && nextChar == '>') return 14;  // >>
        if (currentState == 4 && nextChar == '=') return 16;  // ==
        if (currentState == 4 && nextChar == '>') return 15;  // =>
        if (currentState == 4 && nextChar == '<') return 15;  // =<
        if (currentState == 4 && nextChar == '+') return 20;  // =+
        if (currentState == 5 && nextChar == ':') return 18;  // ::
        if (currentState == 5 && nextChar == '=') return 17;  // =:=
        if (currentState == 7 && nextChar == '+') return 21;  // ++
        if (currentState == 9 && nextChar == '-') return 24;  // --
        if (currentState == 10 && nextChar == '&') return 25; // &&
        if (currentState == 11 && nextChar == '|') return 26; // ||

        // Invalid transitions
        if ((currentState == 7 && nextChar == '=') ||  // += is valid but unhandled
            (currentState == 9 && nextChar == '+') ||  // -+ (invalid)
            (currentState == 6 && nextChar == '=') ||  // *= (invalid in this case)
            (currentState == 11 && nextChar == '=')    // |= (invalid in this case)
            ) return -1;
        return nextState;
    }


    bool isKeyword(const string& token)
    {
        return keywords.find(token) != keywords.end();
    }

    void processToken(const string& token, int lineNum)
    {
        bool isItAKeyword = false, isId = false, isNum = false, isPunc = false, isOp = false;
        if (token.empty())
        {
            cout << "No Tokens\n";
            tokenFile << "No Tokens\n";
            return;
        }
        // cout << "tokens: " << token << " at line " << lineNum << "\n";

        // Keywords
        string keywordPart = token;
        char lastChar = '\0';
        // Check if the last character is non-alphabetic
        if (!isalpha(token.back()))
        {
            keywordPart = token.substr(0, token.length() - 1); // Extract all except the last character
            lastChar = token.back();                          // Store the last character
        }
        if (isKeyword(keywordPart))
        {
            nKeywords++;
            isItAKeyword = true;
            cout << "KEYWORD:" << keywordPart << ":" << lineNum << endl;
            tokenFile << "KEYWORD:" << keywordPart << ":" << lineNum << endl;
            // Process the last character as a separate token, if it's not empty
            if (lastChar != '\0')
            {
                string singleCharToken(1, lastChar);
                processToken(singleCharToken, lineNum);
            }
            return;
        }

        // Identifier
        if (!isItAKeyword)
        {
            int idState = identifierFSM(token); // Process token through the FSM
            if (idState == 3)
            {
                nIdentifiers++;
                isId = true;
                cout << "IDENTIFIER:" << token << ":" << lineNum << endl;
                tokenFile << "IDENTIFIER:" << token << ":" << lineNum << endl;
                return;
            }
        }

        // Number
        if (!isId && !isItAKeyword)
        {
            int numState = numberFSM(token);
            if (numState != -1)
            {
                nNumbers++;
                isNum = true;
                cout << "NUMBER:" << token << ":" << lineNum << endl;
                tokenFile << "NUMBER:" << token << ":" << lineNum << endl;
            }
        }
        if (!isId && !isItAKeyword && !isNum)
        {
            // Punctuation
            if (token.length() == 1)
            {
                int result = punctuationFSM(token); // Process the punctuation using the FSM
                if (result != -1)
                {
                    // If the punctuation is valid
                    nPunctuations++;
                    isPunc = true;
                    cout << "PUNCTUATION:" << token << ":" << lineNum << endl;
                    tokenFile << "PUNCTUATION:" << token << ":" << lineNum << endl;
                    return;
                }
            }
        }

        // Operator
        if (!isId && !isItAKeyword && !isNum && !isPunc)
        {
            int state = 0; // Start state
            size_t i = 0;
            lastChar = '\0';

            while (i < token.size())
            {
                char currentChar = token[i];
                char nextChar = (i + 1 < token.size()) ? token[i + 1] : '\0';
                int nextState = operatorFSM(currentChar, state, nextChar);
                state = nextState;

                // If we consumed a lookahead character, move forward
                if (nextState >= 15)
                    ++i;
                ++i;
            }

            // Final state validation
            if (state >= 5) { // Valid operator
                string operatorPart = token;

                // Check if the last character is alphabetic or numeric, but only if the token has more than 1 character
                if (token.size() > 2 && isalnum(token.back()))
                {
                    operatorPart = token.substr(0, token.length() - 1); // Extract operator part
                    lastChar = token.back();                            // Store the last character
                }

                // Output the operator
                nOperators++;
                isOp = true;
                cout << "OPERATOR:" << operatorPart << ":" << lineNum << endl;
                tokenFile << "OPERATOR:" << operatorPart << ":" << lineNum << endl;

                // Reprocess the last 
                // character as a separate token
                if (lastChar != '\0') {
                    string singleCharToken(1, lastChar);
                    processToken(singleCharToken, lineNum);
                }
            }
        }

        // Error File Generation
        if (!isId && !isItAKeyword && !isNum && !isPunc && !isOp)
        {
            nInvalid++;
            cout << "Lexical_error:" << token << ":" << lineNum << endl;
            errorFile << "Lexical_error:" << token << ":" << lineNum << endl;
        }
    }
};

#endif // LEXICAL_H