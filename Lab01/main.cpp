/* File: Lab01.cpp
 * ---------------------------
 * By           : Jingxin Zhu
 * Last Modified: 06/07/2014
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <map>
#include <algorithm>
//#include <stdlib.h>

using namespace std;

struct DefList{
    int defcnt;
    vector<string> defSymList;
    vector<int> defAddrList;
};

struct ProgramText{
    int codecnt;
    vector<string> typeList;
    vector<string> instrList;
};

void readFile(char* argv[]);
bool isNumber(int tokenIndex);
bool isSymbol(int tokenIndex);
bool parseDefList(int module, vector<DefList>& defListVec);
bool parseUseList(vector< vector<string> >& useListVec);
bool parseProgramText(int moduleCt, int& programSum, vector<ProgramText>& progListVec);
void printSymbolTable(vector<DefList>& defListVec);
void E_instruction(int type, vector<string>& useList);
void R_instruction(int useIndex, int moduleCt);
void A_instruction(int useIndex); 
void I_instruction(int useIndex); 
int parseInstruction(string token);
string getInstr(int num);
string getOrder(int index); 
void printWarning(vector< vector<string> >& useListVec);
void printModuleWarning(vector<string>& useList, int moduleCt);
void printDefWarning(vector<DefList>& defListVec, vector<ProgramText>& progListVec);
void printSyntaxErr(int index, int kind);

vector<string> tokenVec;  // all tokens 
vector<int> rowVec;    // line number of each token
vector<int> colVec;  // store each cols
vector<string> symbolTable;  // for print

vector<int> baseAddList;
vector<string> defShowup;   // store all tokens that have been tested
    
vector<string> typeList;   // all IAER
vector<string> instruList; // all 1000

map<string, int> defMap;  // <defSym, moduleCt>
map<string, int> defSymMap; // <defSym, OccurCt>
map<string, int> symValueMap; //<symbol, value>
map<string, int> useCtMap;   // <useSym, OccurCt>
map<string, int> symModMap; // <TableSym, ModuleCt>
map<int,int> ModSizeMap;   // <ModuleCt, ModuleSize>

int tokenIndex = 0;
int tokenNum;  // The number of total tokens in the file

int main (int argc, char* argv[]){

    if (argc != 2) {
        cout << "Unexpected input format." << endl;
        exit(0);
    }
    readFile(argv);

    tokenNum = tokenVec.size(); // number of all tokens
    int moduleCt = 0;   // counter for nudule number
    int programSum = 0; // sum of programs in all modules
    vector<DefList> defListVec;
    vector< vector<string> > useListVec;
    vector<ProgramText> progListVec;

    /*       Pass One         */
    baseAddList.push_back(0);

    while( tokenIndex < tokenNum ) {
        if( !parseDefList(moduleCt, defListVec) ) {
            return 1;
        }
        if ( !parseUseList(useListVec) ) {
            return 1;
        }	
        if ( !parseProgramText(moduleCt, programSum, progListVec) ) {
            return 1;
        }	
        tokenIndex++;
        moduleCt++;
        baseAddList.push_back(typeList.size());
    }
    printDefWarning(defListVec,progListVec);
    printSymbolTable(defListVec);

    /*       Pass Two         */
    int moduleNum = defListVec.size();
    moduleCt = 0;
    int useIndex = 0;
    //Module by module
    cout << "Memory Map" << endl;
    while( moduleCt < moduleNum ) {
        vector<string> useList = useListVec[moduleCt]; 
        int size = useList.size();
        for (int i = 0; i < size; i++){
            map<string, int>::const_iterator iter = useCtMap.find( useList[i] );
            if ( iter == useCtMap.end()) {
                useCtMap.insert(make_pair(useList[i], 0));
            }
        }

        for (int i = 0; i < progListVec[moduleCt].codecnt; i++) {
            string type = typeList[useIndex];
            if (type == "A")  	
                A_instruction(useIndex);
            if (type == "I")  	
                I_instruction(useIndex);
            if (type == "E")
                E_instruction(useIndex, useList);
            if (type == "R")
                R_instruction(useIndex, moduleCt);
            useIndex++;
        }
        printModuleWarning(useList, moduleCt);
        moduleCt++;
    }

    printWarning(useListVec);

    return 0;
}


/******************************/
/*     Private Functions      */
/******************************/
bool isNumber(int tokenIndex) {
    if (tokenIndex == tokenNum) {
        // Miss token, no token left to be read
        return false;
    } else {
        // Not a pure number
        string token = tokenVec[tokenIndex];
        int len = token.length();
        for (int i = 0; i < len; i++) {
            char ch = token[i];
            if ( ch < '0' || ch > '9') {
                return false;
            }
        }
        return true;
    }
}

bool isSymbol(int tokenIndex) {
    if ( tokenIndex == tokenNum ) {
        return false;
    } else {
        // Unexpected token, not of the form [a-Z][a-Z0-9]*
        string token = tokenVec[tokenIndex];
        char ch = token[0];
        if ( isalpha(ch) ) {
            int len = token.length();
            for (int i = 0; i < len; i++) {
                ch = token[i];
                if ( !isalnum(ch)) return false;
            }
            return true;		
        } else {
            return false;
        }
    }
}

bool isType(int tokenIndex) {
    if (tokenIndex == tokenNum){
        return false;
    } else {
        string token = tokenVec[tokenIndex];
        return (token == "I") || (token =="A") 
            || (token=="E") || (token=="R");
    }
}

void printSymbolTable(vector<DefList>& defListVec) {
    cout << "Symbol Table" << endl;
    DefList deflist;
    int size = defListVec.size();
    for (int i = 0; i < size; i++) {
        deflist = defListVec[i];
        if ( deflist.defcnt != 0 ) {
            int symsize = deflist.defSymList.size();
            for (int j = 0; j < symsize; j++) {
                string token = deflist.defSymList[j];
                int value = baseAddList[defMap[token]] + deflist.defAddrList[j];
                symValueMap.insert(make_pair(token, value));
                if ( defSymMap[token] == 1) {
                    // if a symbol is defined exactly once
                    cout << token << "=" << value << endl;
                } else {
                    cout << token << "=" << value
                        << " Error: This variable is multiple times defined; first value used"<< endl;
                }
            }
        }
    }
    cout << endl;
}

void printDefWarning(vector<DefList>& defListVec, vector<ProgramText>& progListVec) { 
    int count = 0;
    int moduleNum= defListVec.size();
    DefList deflist;
    for (int i = 0; i < moduleNum; i++) {
        deflist = defListVec[i];
        int num = deflist.defcnt;
        // compare each address in the def list to code count	
        for (int j = 0; j < num; j++) {
            string token = deflist.defSymList[j];
            int address = deflist.defAddrList[j];
            if (find(defShowup.begin(), defShowup.end(), token)==defShowup.end()) {
                //if (address >= progCntList[i]) {
                if (address >= progListVec[i].codecnt) {
                    cout << "Warning: Module " << i+1 << ": " 
                        << token << " to big " << address << " (max=" 
                        << progListVec[i].codecnt - 1
                        << ") assume zero relative" << endl;
                    deflist.defAddrList[j] = 0;
                }
                defShowup.push_back(token);
            }
            count++;
        }
    }
}

int parseInstruction(string token) {
    int len = token.length();
    if ( len > 4 ) {
        // if instruction's length > 4, treat as 9999
        return 9999;
    } else {
        string result;
        for (int i = 0; i < (4 - len); i++) {
            result += "0";
        }
        result += token;
        return atoi(result.c_str());
    }
}

string getInstr(int number) {
    string num;
    stringstream ss;
    ss << number;
    ss >> num;
    int length = num.length();
    string result;
    for (int i = 0; i < (4 - length);i++){
        result += "0";
    }
    result += num;
    return result;
}

string getOrder(int index) {
    string result;
    string s;
    stringstream ss;
    ss << index;
    ss >> s;
    int length = s.length();
    for (int i = 0; i < (3 - length); i++) {
        result += "0";
    }
    result += s;
    return result;
}

/* Print warning if a symbol is defined in definition list,
 * but is not ever used in use list.
 */
void printWarning(vector< vector<string> >& useListVec) {
    cout << endl;
    int size = symbolTable.size();
    for (int i = 0; i < size; i++ ) {
        bool found = false;
        string symbol = symbolTable[i];
        int useNum = useListVec.size();
        for (int j = 0; j < useNum; j++) {
            vector<string> useList = useListVec[j];   
            if (find(useList.begin(), useList.end(), symbol) 
                 != useList.end()){
                found = true;
                break;
            }
        }
        if ( found == false ) {
            printf("Warning: Module %d : %s %s\n",
                    defMap[symbol] + 1, symbol.c_str(), 
                    "was defined but never used");
        }
    }
}

void printModuleWarning(vector<string>& useList, int moduleCt) {
    int size = useList.size();
    for (int i=0; i<size; i++) {
        string token = useList[i];
        if ( useCtMap[token] == 0) {
            printf("Warning: Module %d : %s %s\n",
                    moduleCt + 1, token.c_str(),
                    "appeared in the uselist but was not actually used");
        }
    }
}

void readFile(char* argv[]) {
    ifstream infile (argv[1], ios::binary);
    if (infile.is_open()) {
        vector<int> lenList;   // Store the length of each line
        string line;
        int row = 0;
        while ( getline(infile, line)) { 
            row++;
            lenList.push_back(line.length());
            stringstream input(line);
            int col = -1;
            for (string token; input >> token;){
                col = line.find(token, col+1);
                tokenVec.push_back(token);  
                rowVec.push_back(row);
                colVec.push_back(col+1);
            }
        }
        // record end position of input file
        int endRow = row; 
        int endCol = lenList.back() + 1;	
        rowVec.push_back(endRow);
        colVec.push_back(endCol);
        infile.close();
    } else {
        cout << "Error opening file." << endl;
    }
}

void E_instruction(int useIndex, vector<string>& useList) {
    string token = instruList[useIndex];
    int length = token.length();
    if (length > 4) {
        cout << getOrder(useIndex) << ": " << 9999 
            << " Error: Illegal opcode; treated as 9999" << endl;
    } else { 
        int instr = parseInstruction(token);
        int opcode = instr / 1000;
        int operand = instr % 1000;	
        // If the symbol is in the use list but not defined in the symbol table
        int size = useList.size();
        if (operand > size-1 ) {
            cout << getOrder(useIndex) << ": " << getInstr(instr) <<  
                " Error: External address exceeds length of uselist; treated as immediate" << endl;
        } else { 
            string token = useList[operand];
            map<string, int>::const_iterator iter = defSymMap.find(token);
            operand = symValueMap[token];	
            if ( iter == defSymMap.end() ) {
                cout << getOrder(useIndex) << ": "<< getInstr(opcode * 1000) 
                    << " Error: " << token << " is not defined; zero used" << endl;
                //symbolTable.push_back(token);
                useCtMap[token] += 1;
            } else {
                useCtMap[token] += 1; 
                cout << getOrder(useIndex) << ": " 
                    << getInstr(opcode * 1000 + operand) << endl;
            }
        }
    }
}

void A_instruction(int useIndex) {
    string token = instruList[useIndex];
    if (token.length() > 4) {
        cout << getOrder(useIndex) << ": " << 9999 
            << " Error: Illegal opcode; treated as 9999" << endl;
    } else { 
        int instr = parseInstruction(token);
        int opcode = instr / 1000;
        int operand = instr % 1000;
        if ( operand > 511 ) {
            cout << getOrder(useIndex) << ": " << getInstr(opcode * 1000) << 
                " Error: Absolute address exceeds machine size; zero used" << endl;
        } else {
            cout << getOrder(useIndex) << ": " << getInstr(instr) << endl;
        }
    }
}

void I_instruction(int useIndex) {
    string token = instruList[useIndex];
    if ( token.length() > 4 ) {
        cout << getOrder(useIndex) << ": " << 9999 << 
            " Error: Illegal immediate value; treated as 9999" << endl;	
    } else {
        int instr = parseInstruction(token);
        cout << getOrder(useIndex) << ": " << getInstr(instr) << endl;
    }
}

void R_instruction(int useIndex, int moduleCt){
    string token = instruList[useIndex];
    if (token.length() > 4) {
        cout << getOrder(useIndex) << ": " << 9999 
            << " Error: Illegal opcode; treated as 9999" << endl;
    } else { 
        int instr = parseInstruction(token);
        int opcode = instr / 1000;
        int operand = instr % 1000;
        if (operand > ModSizeMap[moduleCt]) {
            cout << getOrder(useIndex) << ": "
                << getInstr(opcode * 1000 + baseAddList[moduleCt]) 
                << " Error: Relative address exceeds module size; zero used" <<	endl;
        } else {
            cout << getOrder(useIndex) << ": " 
                << getInstr(instr + baseAddList[moduleCt]) << endl;
        }
    }
}

bool parseDefList(int module, vector<DefList>& defListVec) {
    // test def counter: is defCnt a number? 
    DefList deflist;
    if (!isNumber(tokenIndex)){
        printSyntaxErr(tokenIndex, 1);
        return false;
    } else {
    // test def counter: is defCnt < MAX_DEF_NUM (16)?
        int defCnt = atoi(tokenVec[tokenIndex].c_str());
        deflist.defcnt = defCnt;
        
        if (defCnt > 16) {
            // TO_MANY_DEF_IN_MODULE: 
            printSyntaxErr(tokenIndex, 2);
            return false;
        } else {
            // test def symbol: is defsym a valid symbol?
            for (int i = 0; i < defCnt; i++) {		
                tokenIndex++;
                // SYM_EXPECTED: if miss token or unexpected token
                if( !isSymbol(tokenIndex) ) {
                    printSyntaxErr(tokenIndex, 3);
                    return false;
                } else {
            // test def symbol: is defsym too long? (> 16)
                    if (tokenVec[tokenIndex].length() > 16) {
                        printSyntaxErr(tokenIndex, 5);
                        return false;
                    }
                    string token = tokenVec[tokenIndex];
                    deflist.defSymList.push_back(token);

                    // store the module number of each def symbol
                    map<string, int>::const_iterator iter = defMap.find(token);
                    if ( iter == defMap.end() ) {
                        defMap.insert(make_pair(token, module));
                    }

                    // store the times that one symbol occurs 
                    defSymMap[token] += 1;
                    if (defSymMap[token] == 1) {
                        symbolTable.push_back(token);
                        symModMap.insert(make_pair(token, module));
                    }

                    tokenIndex++;	
                    if ( !isNumber(tokenIndex)) {
                        printSyntaxErr(tokenIndex, 1);
                        return false;
                    } else {
                        deflist.defAddrList.push_back(atoi(tokenVec[tokenIndex].c_str()));
                    } // "isNumber" if-else ends here
                } // "isSymbol" if-else ends here
            } // for-loop ends here
        }
    } // "isNumber" if-else ends here
    defListVec.push_back(deflist);
    return true;
}

bool parseUseList(vector< vector<string> >& useListVec){
    vector<string> useList;
    tokenIndex++;
    if (!isNumber(tokenIndex)){
        // test uselist number: is useCnt a valid number? 
        printSyntaxErr(tokenIndex, 1);
        return false;
    } else {
        int useCnt = atoi(tokenVec[tokenIndex].c_str());
        if (useCnt > 16) {
            printSyntaxErr(tokenIndex, 4); // TO_MANY_USE_IN_MODULE
            return false;
        } else {
            for (int i = 0; i < useCnt; i++) {		
                tokenIndex++;
                if( !isSymbol(tokenIndex) ) {
                    // test use symbol: is a valid symbol?
                    printSyntaxErr(tokenIndex, 2); // SYM_EXPECTED
                    return false;
                } else {
                    if ( tokenVec[tokenIndex].length() > 16 ) {
                        printSyntaxErr(tokenIndex, 5); // SYM_TOLONG
                        return false;
                    }
                    useList.push_back(tokenVec[tokenIndex]);
                } // "isSymbol" if-else ends here
            } // for-loop ends here
        }
    } // "isNumber" if-else ends here
    useListVec.push_back( useList );
    return true;
}

bool parseProgramText(int moduleCt, int& programSum, vector<ProgramText>& progListVec) {
    tokenIndex++;
    ProgramText pt;
    if (!isNumber(tokenIndex)){
        // test progCnt: is progCnt is a valid number?
        printSyntaxErr(tokenIndex, 1);
        return false;
    } else {
        int progCt= atoi(tokenVec[tokenIndex].c_str());
        if ( progCt > (512 - programSum)) {
            printSyntaxErr(tokenIndex, 6);
            return false;
        } else {
            programSum += progCt;
            pt.codecnt = progCt;
            ModSizeMap.insert(make_pair(moduleCt, progCt)); 
            for (int i = 0; i < progCt; i++) { 
                tokenIndex++;
                // Miss token or Unexpected token
                if( !isType(tokenIndex) ) {
                    printSyntaxErr(tokenIndex, 7); // ADDR_EXPECTED
                    return false;
                } else {
                    typeList.push_back(tokenVec[tokenIndex]);
                    tokenIndex++;	
                    if ( !isNumber(tokenIndex)) {
                        printSyntaxErr(tokenIndex, 1);
                        return false;
                    } else {
                        instruList.push_back( (tokenVec[tokenIndex]) );
                    }
                }
            }	
        }
    }	
    progListVec.push_back(pt);
    return true;
}

void printSyntaxErr(int index, int kind) {
    string err;
    if (kind == 1) {
        err = "NUM_EXPECTED";
    } else if (kind == 2) {
        err = "TO_MANY_DEF_IN_MODULE";
    } else if (kind == 3) {
        err = "SYM_EXPECTED";
    } else if (kind == 4) {
        err = "TO_MANY_USE_IN_MODULE";
    } else if (kind == 5) {
        err = "SYM_TOLONG";
    } else if (kind == 6) {
        err = "TO_MANY_INSTR";
    } else {
        err = "ADDR_EXPECTED";
    }
    printf("Parse Error line %d offset %d: %s\n",
        rowVec[index], colVec[index], err.c_str());
}