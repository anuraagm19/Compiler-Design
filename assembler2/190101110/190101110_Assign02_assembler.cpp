/*
CS348 Programming  Assignmment 2
Anuraag Mahajan 1901011110

 Commands to execute program: (LINUX g++)
 $ g++ assembler.cpp
 $ ./a.out <inputfile>

 Output file: output.txt
 intermediate.txt : Pass1 will create an intermediate file named intermediate.txt containing assigned addresses to all statements in the program
 modified.txt: extra file used as utility
*/

#include <bits/stdc++.h>
using namespace std;

/////////////////////////////////////////////////////  ---------- PARAMETERS ------------ ////////////////////////////////////////////

// I/O streams
ifstream input;
ofstream intermediate;
ifstream intermediate_in;
ofstream output;
fstream modif_util;


/////  data structures for holding LOCTRation,symbols,ext,literals, mnemonics, etc.

unordered_set<string> format2 = {"COMPR", "CLEAR", "TIXR"};

unordered_map<string, int> REG {
    {"A", 0},
    {"X", 1},
    {"L", 2},
    {"B", 3},
    {"S", 4},
    {"T", 5},
    {"F", 6},
};
unordered_map<string, string> OPTAB = {
    {"LDA", "00"},
    {"LDL", "08"},
    {"LDX", "04"},
    {"LDB", "68"},
    {"LDT", "74"},
    {"STA", "0C"},
    {"STL", "14"},
    {"STX", "10"},
    {"LDCH", "50"},
    {"STCH", "54"},
    {"ADD", "18"},
    {"SUB", "1C"},
    {"MUL", "20"},
    {"DIV", "24"},
    {"COMP", "28"},
    {"COMPR", "A0"},
    {"CLEAR", "B4"},
    {"J", "3C"},
    {"JLT", "38"},
    {"JEQ", "30"},
    {"JGT", "34"},
    {"JSUB", "48"},
    {"RSUB", "4C"},
    {"TIX", "2C"},
    {"TIXR", "B8"},
    {"TD", "E0"},
    {"RD", "D8"},
    {"WD", "DC"},
};

struct container
{
    string tag;
    int a[3];
};

struct extsym
{
    string symbol;
    int type;
    int control_section;
};

container LOCTAB[50];
container SYMTAB[50];
extsym EXTAB[50];

unordered_map<string, container> LITTAB;



/////////// important paramenters and utility variables
string buffer, OPERAND, OPCODE, LABEL;

int INDEX_SYMTAB, SIZE_SYMTAB, inSYMTAB;
int inOPTAB;
int SIZE_LOCTAB, inLOCTAB, INDEX_LOCTAB;
int INDEX_EXT, SIZE_EXT;

int index_buffer, CURRENT_SECT, LOCTR;
int OUT_LINE, INDEX_LINE, INDEX_RECORD;
int DEF_IND, MODIF_IND;

bool is_extended_format;
string record, ONE_LINE, TEXT_RECORD, ONE_LINE_HOLDER;
string DEF_RECORD, MODIF_RECORD;



/////////////////////////////////////////////////////  ---------- HELPER FUNCTIONS ------------ ////////////////////////////////////////////


// gives appropiate flags n,i,x,e,b,p to format object codes supporting sic/xe architecture
int getSetBit(int type, char flag)
{
    if (type == 3)
    {
        if (flag == 'n')
            return 1 << 17;
        if (flag == 'i')
            return 1 << 16;
        if (flag == 'b')
            return 1 << 14;
        if (flag == 'x')
            return 1 << 15;
        if (flag == 'p')
            return 1 << 13;
        if (flag == 'e')
            return 1 << 12;
    }
    if (type == 4)
    {
        if (flag == 'n')
            return 1 << 25;
        if (flag == 'i')
            return 1 << 24;
        if (flag == 'b')
            return 1 << 22;
        if (flag == 'x')
            return 1 << 23;
        if (flag == 'p')
            return 1 << 21;
        if (flag == 'e')
            return 1 << 20;
    }
    return -1;
}

// search SYTMTAB for a symbol
void findSYMTAB(string &str)
{
    INDEX_SYMTAB = -1;
    inSYMTAB = 0;
    int i = 0;
    while (i < SIZE_SYMTAB)
    {
        if (SYMTAB[i].tag == str && SYMTAB[i].a[2] == CURRENT_SECT)
        {
            inSYMTAB = 1;
            INDEX_SYMTAB = i;
            return;
        }
        i++;
    }
}

// add new symbol to SYMTAB
void insertSYMTAB(string &str)
{
    inSYMTAB = 0;
    INDEX_SYMTAB = SIZE_SYMTAB;
    SIZE_SYMTAB++;
    SYMTAB[INDEX_SYMTAB].a[0] = LOCTAB[CURRENT_SECT].a[0];
    SYMTAB[INDEX_SYMTAB].a[1] = 0;
    SYMTAB[INDEX_SYMTAB].a[2] = CURRENT_SECT;
    SYMTAB[INDEX_SYMTAB].tag = str;
}

// search the EXT table for definition and reference records
void findEXTAB(string str, bool is_extdef)
{
    bool EXT_FOUND = 0;
    INDEX_EXT = -1;
    string token = "";
    istringstream iss(str);
    while (getline(iss, token, ','))
    {
        int i = 0;
        while (i < SIZE_EXT)
        {
            if (EXTAB[i].symbol == token && EXTAB[i].control_section == CURRENT_SECT)
            {

                EXT_FOUND = 1;
                INDEX_EXT = i;
                break;
            }
            i++;
        }

        if (!EXT_FOUND)
        {
            SIZE_EXT++;
            INDEX_EXT++;
            EXTAB[INDEX_EXT].symbol = token;
            EXTAB[INDEX_EXT].control_section = CURRENT_SECT;
            EXTAB[INDEX_EXT].type = is_extdef;
        }
    }
}

// utitilty function for space separated words
void iterate(string &str)
{

    while (buffer[index_buffer] != ' ' && buffer[index_buffer] != '\t' && buffer[index_buffer] != '\n')
    {
        str += buffer[index_buffer++];
    }
}

// ignore spaces in line
void ignoreSpaces()
{
    while (buffer[index_buffer] == ' ' || buffer[index_buffer] == '\t')
    {
        index_buffer++;
    }
}

//  string to hexadecimal integer
int stringToHex(string &str)
{
    char *p;
    return strtol(str.c_str(), &p, 16);
}

// Calculate lenght of operands and add to LOCTR
int getOperandSize(string &str)
{
    int i = 0;
    int len = str.length();
    if (str[0] == 'X' && str[1] == '\'' && str[len - 1] == '\'')
    {
        i = 1;
        return i;
    }
    else if (str[0] == 'C' && str[1] == '\'' && str[len - 1] == '\'')
    {
        i = 2;
        while (i <= str.size())
        {
            if (str[i] == '\'')
            {
                i = i - 2;
                break;
            }
            i++;
        }
        return i;
    }
    return -1;
}

// formatted printing
void padZeros(ofstream &file_pointer, int set)
{
    file_pointer << setfill('0') << setw(set) << right << uppercase << hex;
}

// formatted printing
void addSpaces(string &str, int sz)
{
    for (int i = sz; i < 6; i++)
    {
        str += ' ';
    }
}

// finding the format type from sic xe for creating the object code as per instructions
int getType(string &opc)
{

    if (format2.find(opc) != format2.end())
        return 2;
    if (opc[0] == '+')
    {
        return 4;
    }
    if (OPTAB.find(opc) == OPTAB.end())
    {
        return 0;
    }

    return 3;
}

// formatted printing 
void addPad(ofstream &file_pointer)
{
    string newLABEL = LABEL;
    for (int i = LABEL.size(); i < 8; i++)
    {
        newLABEL += ' ';
    }
    string newOperand = OPERAND;
    for (int i = OPERAND.size(); i < 8; i++)
    {
        newOperand += ' ';
    }
    string newOpcode = OPCODE;
    for (int i = OPCODE.size(); i < 8; i++)
    {
        newOpcode += ' ';
    }
    file_pointer << newLABEL << newOpcode << newOperand << "\n";
}

// fetches space separated words from a input file and assigns them to appropriate parameters - LABEL, OPERAND, OPCODE
void parseLinePass1(ifstream &file_pointer)
{
    index_buffer = 0;
    buffer.clear();
    getline(file_pointer, buffer);
    buffer += '\n';
    // ++;
    LABEL = OPCODE = OPERAND = "";
    // Read Label
    iterate(LABEL);
    ignoreSpaces();
    // Read Opcode
    iterate(OPCODE);
    ignoreSpaces();
    // Read Operand
    iterate(OPERAND);
    
}

// fetches space separated words from intermediate.txt and assigns them to appropriate parameters - LABEL, OPERAND, OPCODE, LOCATION
void parseLinePass2(ifstream &file_pointer)
{
    buffer.clear();
    getline(file_pointer, buffer);
    buffer += '\n';
    index_buffer = 0;
    // Read LOCCTR
    string tmp = "";
    iterate(tmp);
    LOCTR = stringToHex(tmp);
    index_buffer += 1;
    LABEL = OPCODE = OPERAND = "";
    // Read Label
    iterate(LABEL);
    ignoreSpaces();
    // Read Opcode
    iterate(OPCODE);
    ignoreSpaces();
    // Read Operand
    iterate(OPERAND);
}

// calculate value of literal (for RESB and RESW)
int getLitVal(string &str)
{
    int val = 0;
    int len = str.length();
    int i = 2;
    if (str[0] == 'X' && str[1] == '\'' && str[len - 1] == '\'')
    {
        string pointer;
        while (i != str.length() - 1)
        {
            pointer += str[i];
            i++;
        }
        val += stringToHex(pointer);
        return val;
    }
    if (str[0] == 'C' && str[1] == '\'' && str[len - 1] == '\'')
    {
        while (i != str.length() - 1)
        {
            val += (int)str[i];
            val <<= 8;
            i++;
        }
        val >>= 8;
        return val;
    }
    return val;
}


// calculates size indicated by operand used to calculate next addrress.
void getNewLoc(int &n)
{
    if (format2.find(OPCODE) != format2.end())
    {
        n += 2;
    }
    else if (inOPTAB)
    {
        is_extended_format ? n += 4 : n += 3;
    }
    else if (OPCODE == "BYTE")
    {
        n += getOperandSize(OPERAND);
    }
    else if (OPCODE == "WORD")
    {
        n += 3;
    }
    else if (OPCODE == "RESW")
    {
        n += 3 * stoi(OPERAND);
    }
    else if (OPCODE == "RESB")
    {
        n += stoi(OPERAND);
    }
}

// parse expression for EQU 
void parseExpression(bool *isref,string &str, bool *kind,int *val)
{
    *kind = false;
    bool found = false;
    string tmp;
    int cnt_rel = 0;
    if (str == "*")
    {
        cnt_rel++;
        *val = LOCTAB[CURRENT_SECT].a[0];
    }
    else
    {
        int prev_sign = 1;
        for (int i = 0; i < str.length(); i++)
        {
            if (str[i] == '-' || str[i] == '+')
            {
                found = false;
                int k = 0;
                while (k < SIZE_SYMTAB)
                {
                    if (SYMTAB[k].tag == tmp && SYMTAB[k].a[2] != CURRENT_SECT)
                    {

                        *isref = true;
                        return;
                    }
                    if (SYMTAB[k].tag == tmp)
                    {
                        *val += prev_sign * SYMTAB[k].a[0];
                        cnt_rel += prev_sign;
                        found = true;
                        break;
                    }
                    k++;
                }
                tmp.clear();
                str[i] == '-' ? prev_sign = -1 : prev_sign = 1;
            }
            else
            {
                tmp += str[i];
            }
        }
        found = false;
        int k = 0;
        while (k < SIZE_SYMTAB)
        {
            if (SYMTAB[k].tag == tmp && SYMTAB[k].a[2] != CURRENT_SECT)
            {

                *isref = true;
                return;
            }
            if (SYMTAB[k].tag == tmp)
            {
                *val += prev_sign * SYMTAB[k].a[0];
                cnt_rel += prev_sign;
                found = true;
                break;
            }
            k++;
        }
    }
    *isref = false;
    if (cnt_rel == 0)
    {
        *kind = true;
    }
}

// Assign the addresses for literals and set LOC accordingly when LTORG is encountered
void printLTORG(ofstream &file_pointer)
{
    for (auto i : LITTAB)
    {
        if (i.second.a[0] == -1)
        {
            string n = i.second.tag;
            LITTAB[n].a[0] = LOCTAB[CURRENT_SECT].a[0];
            padZeros(file_pointer, 4);
            file_pointer << LOCTAB[CURRENT_SECT].a[0] << "\t"
                 << "*\t\t ";
            file_pointer << setfill(' ') << setw(8) << left << n << "\n";
            string dummy = n.substr(1);
            LOCTAB[CURRENT_SECT].a[0] += getOperandSize(dummy);
        }
    }
}

int main(int argc, char **argv)
{
    ofstream util;
    util.open("modified.txt");
    // open required I/O streams
    if (argc != 2)
    {
        cout << "Provide input file name as a single argument\n";
        exit(1);
    }
    string input_file = argv[1];

    input.open(input_file);
    intermediate.open("intermediate.txt");

    if (!input.is_open())
    {
        printf("Cannot open input file\n");
        exit(1);
    }

    // initialize important parameters
    buffer = OPERAND = OPCODE = LABEL = "";
    index_buffer = SIZE_SYMTAB = SIZE_LOCTAB = SIZE_EXT = inSYMTAB = inOPTAB = inLOCTAB = 0;
    INDEX_LOCTAB = INDEX_SYMTAB = INDEX_EXT = -1;
    is_extended_format = false;

    /////////////////////////////////////////////////////  ---------- PASS 1 ------------ ////////////////////////////////////////////


    // read START
    parseLinePass1(input);
    if (OPCODE == "START")
    {
        LOCTAB[CURRENT_SECT].a[1] = LOCTAB[CURRENT_SECT].a[0] = stringToHex(OPERAND);
        LOCTAB[CURRENT_SECT].tag = LABEL;
        padZeros(intermediate, 4);
        intermediate << LOCTAB[CURRENT_SECT].a[0] << "\t";
        addPad(intermediate);
        SIZE_LOCTAB++;
        parseLinePass1(input);
    }
    else
    {
        LOCTAB[CURRENT_SECT].a[1] = LOCTAB[CURRENT_SECT].a[0] = 0;
    }
    while (OPCODE != "END")
    {
        // handle comments and blank line
        if (buffer[0] == '.' || buffer[0] == '\n')
        {
            intermediate << buffer;
            parseLinePass1(input);
            continue;
        }

        if (OPCODE == "EQU")
        {
            bool kind, isref;
            isref = false;
            int val = 0;
            kind = false;
            parseExpression(&isref, OPERAND, &kind, &val); // evaluate the OPERAND
            // search the SYMTAB for the LABEL and add if not present
            findSYMTAB(LABEL);
            if (!inSYMTAB)
                insertSYMTAB(LABEL);
            SYMTAB[INDEX_SYMTAB].a[0] = val;       
            SYMTAB[INDEX_SYMTAB].a[1] = kind;
            padZeros(intermediate, 4);
            intermediate << val << "\t";
            addPad(intermediate);
        }
        else if (OPCODE == "CSECT") 
        {
            LOCTAB[CURRENT_SECT].a[2] = LOCTAB[CURRENT_SECT].a[0] - LOCTAB[CURRENT_SECT].a[1];

            SIZE_LOCTAB++;             // start new section
            CURRENT_SECT++;
            LOCTAB[CURRENT_SECT].a[0] = LOCTAB[CURRENT_SECT].a[1] = 0;
            LOCTAB[CURRENT_SECT].tag = LABEL;
            padZeros(intermediate, 4);
            intermediate << LOCTAB[CURRENT_SECT].a[0] << "\t";
            addPad(intermediate);
        }
        else
        {
            if (LABEL.length())
            {                        
                findSYMTAB(LABEL); // search for symbol in SYMTAB and if not present then add
                if (!inSYMTAB)
                {
                    insertSYMTAB(LABEL);
                }
            }
            string tmp_OPCODE;
            is_extended_format = (OPCODE[0] == '+');
            is_extended_format ? tmp_OPCODE = OPCODE.substr(1) : tmp_OPCODE = OPCODE;

            inOPTAB = !(OPTAB.find(tmp_OPCODE) == OPTAB.end());
            // OPCODE in OPTAB?

            if (OPERAND[0] == '=' && (LITTAB.find(OPERAND) == LITTAB.end())) // literal
            { 
                container l;
                l.tag = OPERAND;
                l.a[0] = -1;
                string tmp = OPERAND.substr(1);
                l.a[1] = getLitVal(tmp);
                l.a[2] = getOperandSize(tmp);
                LITTAB[OPERAND] = l;
            }
            if (OPCODE == "EXTREF" || OPCODE == "EXTDEF") // EXT
            {
                OPCODE == "EXTDEF" ? findEXTAB(OPERAND, true) : findEXTAB(OPERAND, false);
                intermediate << "\t\t";
                addPad(intermediate);
            }
            else if (OPCODE == "LTORG")
            {
                intermediate << "\t\t";
                addPad(intermediate);
                printLTORG(intermediate); 
            }
            else
            {
                padZeros(intermediate, 4);
                intermediate << LOCTAB[CURRENT_SECT].a[0] << "\t";
                addPad(intermediate);
            }
        }
        getNewLoc(LOCTAB[CURRENT_SECT].a[0]); // increment LOCTR
        parseLinePass1(input);                         
    }
    intermediate << "\t\t";
    addPad(intermediate);
    printLTORG(intermediate); // print LTORG lines 

    LOCTAB[CURRENT_SECT].a[2] = LOCTAB[CURRENT_SECT].a[0] - LOCTAB[CURRENT_SECT].a[1];
    input.close();
    intermediate.close();

    /////////////////////////////////////////////////////  ---------- PASS 2 ------------ ////////////////////////////////////////////


    

    // initialize important parameters
    buffer = OPERAND = OPCODE = LABEL = "";
    index_buffer = CURRENT_SECT = inOPTAB = inLOCTAB = inSYMTAB = 0;
    INDEX_SYMTAB = INDEX_LOCTAB = -1;
    is_extended_format = false;


    // use intermediate file created in pass 1
    intermediate_in.open("intermediate.txt");

    
    output.open("output.txt");

    modif_util.open("modified.txt");  // utility file

    parseLinePass2(intermediate_in);

    if (OPCODE == "START")
    {
        parseLinePass2(intermediate_in);
    }

    //print head record
    output << "H";
    output << "^";
    output << setfill(' ') << setw(6) << left << LOCTAB[CURRENT_SECT].tag;
    output << "^";
    padZeros(output, 6);
    output << LOCTAB[CURRENT_SECT].a[1];
    output << "^";
    padZeros(output, 6);
    output << LOCTAB[CURRENT_SECT].a[2] << "\n";

    char buff[100];
    INDEX_RECORD += sprintf(buff, "%s", "T");
    // record += "^";
    record += buff;
    INDEX_RECORD += sprintf(buff, "^%06X", LOCTR);
    // record += "^";
    record += buff;
    int start_LOCTR = LOCTR;

    while (OPCODE != "END")
    {
        OUT_LINE = INDEX_LINE = inOPTAB = inSYMTAB = 0;
        TEXT_RECORD.clear();

        // handel comments or blank lines
        if (buffer[0] == '.' || buffer[0] == '\n')
        {
            parseLinePass2(intermediate_in);
            continue;
        }

        int format = getType(OPCODE);
        if (LABEL == "*")
        { // creating the OPCODE as per diff formats of OPCODE
            if (LITTAB.find(OPCODE) != LITTAB.end())
            {
                OUT_LINE = LITTAB[OPCODE].a[1];
            }
            // TEXT_RECORD = "^";
            sprintf(buff, "^%X", OUT_LINE);
            // TEXT_RECORD += "^";
            TEXT_RECORD = buff;
        }
        else if (format == 4) // handling 4 BYTE instructions
        {
            MODIF_RECORD = "M";
            MODIF_IND = sprintf(buff, "^%06X^05^+", LOCTR + 1);
            // MODIF_RECORD += "^";
            MODIF_RECORD += buff;
            string OPCODE_1 = OPCODE.substr(1);
            OUT_LINE = stringToHex(OPTAB[OPCODE_1]);
            OUT_LINE <<= 24;
            OUT_LINE += getSetBit(4, 'n') + getSetBit(4, 'i') + getSetBit(4, 'e');
            if (OPERAND.length())
            {
                if ((int)('X' - OPERAND[OPERAND.length() - 1]) == 0 && OPERAND[OPERAND.length() - 2] == ',')
                {
                    // +OP m,X
                    OPERAND[OPERAND.length() - 2] = '\0';
                    OUT_LINE += getSetBit(4, 'x');
                    char c[100];
                    int i = 0;
                    while (i < OPERAND.size())
                    {
                        c[i] = OPERAND[i];
                        i++;
                    }
                    c[i] = '\0';
                    MODIF_IND += sprintf(buff, "%-6s", c);
                    // MODIF_RECORD += "^";
                    MODIF_RECORD += buff;
                    findSYMTAB(OPCODE);
                    if (inSYMTAB)
                    {
                        INDEX_LINE = (SYMTAB[INDEX_SYMTAB].a[0] - (LOCTR + 3)) & ((1 << 12) - 1);
                        OUT_LINE += INDEX_LINE;
                    }
                }
                else
                {
                    findSYMTAB(OPERAND);
                    string tmp = OPERAND;
                    addSpaces(tmp, tmp.size());
                    // MODIF_RECORD += "^";
                    MODIF_RECORD += tmp;

                    if (inSYMTAB)
                    {
                        INDEX_LINE += SYMTAB[INDEX_SYMTAB].a[0];
                    }
                    else
                    {
                        int i = 0;
                        while (i < SIZE_EXT)
                        {
                            if (EXTAB[i].symbol == OPERAND && EXTAB[i].control_section == CURRENT_SECT)
                            {
                                INDEX_LINE = 0;
                            }
                            i++;
                        }
                    }
                }
                modif_util << MODIF_RECORD << "\n";
            }
            INDEX_LINE &= ((1 << 20) - 1);
            OUT_LINE += INDEX_LINE;
            // TEXT_RECORD = "^";
            sprintf(buff, "^%08X", OUT_LINE);
            // TEXT_RECORD += "^";
            TEXT_RECORD = buff;
        }
        else if (format == 2) // handling 2 BYTE instructions
        {

            OUT_LINE = stringToHex(OPTAB[OPCODE]);
            OUT_LINE <<= 8;
            string reg1, reg2;
            reg1 = OPERAND[0];
            OUT_LINE += (REG[reg1] << 4);
            if (OPERAND[1] == ',')
            {
                reg2 = OPERAND[2];
                OUT_LINE += REG[reg2];
            }
            // TEXT_RECORD = "^";
            sprintf(buff, "^%04X", OUT_LINE);
            TEXT_RECORD = buff;
        }
        else if (format == 3)  // handling 3 BYTE instructions
        {

            OUT_LINE = stringToHex(OPTAB[OPCODE]);
            OUT_LINE <<= 16;

            if (OPERAND.length())
            {
                if (OPERAND[0] == '@')
                { // Indirect

                    string OPERAND_1 = OPERAND.substr(1);
                    findSYMTAB(OPERAND_1);
                    if (inSYMTAB)
                    { // OP @m
                        INDEX_LINE = (SYMTAB[INDEX_SYMTAB].a[0] - (LOCTR + 3));
                        INDEX_LINE = INDEX_LINE & ((1 << 12) - 1);
                        OUT_LINE += INDEX_LINE + getSetBit(3, 'p') + getSetBit(3, 'n');
                    }
                    else
                    {
                        // OP @c
                        OUT_LINE += stringToHex(OPERAND_1) + getSetBit(3, 'n');
                    }
                }
                else if (OPERAND[0] == '#')
                { // Immediate
                    string OPERAND_1 = OPERAND.substr(1);
                    findSYMTAB(OPERAND_1);
                    if (inSYMTAB)
                    { // OP #m
                        OUT_LINE += SYMTAB[INDEX_SYMTAB].a[0] - (LOCTR + 3) + getSetBit(3, 'i') + getSetBit(3, 'p');
                    }
                    else
                    { // OP #c
                        OUT_LINE += stringToHex(OPERAND_1) + getSetBit(3, 'i');
                    }
                }
                else if (OPERAND[0] == '=')
                {
                    if (LITTAB.find(OPERAND) != LITTAB.end())
                    {
                        INDEX_LINE = (LITTAB[OPERAND].a[0] - (LOCTR + 3)) & ((1 << 12) - 1);
                        OUT_LINE += INDEX_LINE;
                        OUT_LINE += getSetBit(3, 'p') + getSetBit(3, 'n') + getSetBit(3, 'i');
                    }
                    else
                    {
                        OUT_LINE += getSetBit(3, 'p') + getSetBit(3, 'n') + getSetBit(3, 'i');
                    }
                }
                else if ((int)('X' - OPERAND[OPERAND.length() - 1]) == 0)
                { // OP m,X
                    if (OPERAND[OPERAND.length() - 2] == ',')
                    {
                        OUT_LINE += getSetBit(3, 'n') + getSetBit(3, 'i') + getSetBit(3, 'x') + getSetBit(3, 'p');
                        OPERAND[OPERAND.length() - 2] = '\0';
                        findSYMTAB(OPCODE);
                        if (inSYMTAB)
                        {
                            INDEX_LINE = (SYMTAB[INDEX_SYMTAB].a[0] - (LOCTR + 3)) & ((1 << 12) - 1);
                            OUT_LINE += INDEX_LINE;
                        }
                    }
                    else
                    {
                        findSYMTAB(OPERAND);
                        OUT_LINE += getSetBit(3, 'p') + getSetBit(3, 'n') + getSetBit(3, 'i');
                        if (inSYMTAB)
                        { // OP m
                            INDEX_LINE = (SYMTAB[INDEX_SYMTAB].a[0] - (LOCTR + 3)) & ((1 << 12) - 1);
                            OUT_LINE += INDEX_LINE;
                        }
                    }
                }
                else
                { // simple
                    findSYMTAB(OPERAND);
                    OUT_LINE += getSetBit(3, 'p') + getSetBit(3, 'n') + getSetBit(3, 'i');
                    if (inSYMTAB)
                    { // OP m
                        INDEX_LINE = (SYMTAB[INDEX_SYMTAB].a[0] - (LOCTR + 3)) & ((1 << 12) - 1);
                        OUT_LINE += INDEX_LINE;
                    }
                }
            }
            else
            {
                OUT_LINE += getSetBit(3, 'i') + getSetBit(3, 'n');
            }
            // TEXT_RECORD = "^";
            sprintf(buff, "^%06X", OUT_LINE);
            TEXT_RECORD = buff;
        }
        else if (OPCODE == "BYTE")
        {
            char pointer[32];
            OUT_LINE = 0;
            int tmp_index = 0;
            if ((int)(OPERAND[0] - 'C' == 0) && OPERAND[1] == '\'')
            {
                int i = 2;
                while (i <= OPERAND.length() - 2)
                {
                    OUT_LINE += (int)OPERAND[i];
                    OUT_LINE <<= 8;
                    i++;
                }
                OUT_LINE >>= 8;
                sprintf(buff, "^%X", OUT_LINE);
                // TEXT_RECORD = "^";
                TEXT_RECORD = buff;
            }
            else if ((int)(OPERAND[0] - 'X' == 0) && OPERAND[1] == '\'')
            {
                int i = 2;
                while (i <= OPERAND.length() - 2)
                {
                    pointer[tmp_index++] = OPERAND[i];
                    i++;
                }
                pointer[tmp_index] = '\0';
                string c = pointer;
                OUT_LINE += stringToHex(c);
                TEXT_RECORD += "^";
                TEXT_RECORD += pointer;
            }
        }
        else if (OPCODE == "CSECT") // new section
        { 

            INDEX_LOCTAB = -1;
            

            for (int i = 0; i < SIZE_LOCTAB; i++) // searching the LOCTR table
            {
                if (LOCTAB[i].tag == LABEL)
                {
                    inLOCTAB = 1;
                    INDEX_LOCTAB = i;
                    break;
                }
            }
            if (inLOCTAB)
            {
                // print modification records and end

                INDEX_RECORD += sprintf(buff, "^%02X", (int)(ONE_LINE.length() / 2));
                // record += "^";
                record += buff;
                string tmp = ONE_LINE;
                addSpaces(tmp, ONE_LINE.size());
                // record += "^";
                record += ONE_LINE_HOLDER;
                if (ONE_LINE.length())
                {
                    output << record << "\n";
                }
                modif_util.close();
                modif_util.open("modified.txt", ios::in);

                while (getline(modif_util, MODIF_RECORD))
                {
                    output << MODIF_RECORD << "\n";
                }
                modif_util.close();
                modif_util.open("modified.txt", ios::out);
                MODIF_RECORD.clear();
                MODIF_IND = 0;

                if (CURRENT_SECT == 0)
                {
                    output << "E";
                    output << "^";
                    padZeros(output, 6);
                    output << LOCTAB[CURRENT_SECT].a[1] << "\n\n";
                }
                else                                       
                {
                    output << "E\n\n";
                }
                ONE_LINE_HOLDER.clear();
                ONE_LINE.clear();
                CURRENT_SECT++;
                start_LOCTR = LOCTAB[CURRENT_SECT].a[1];
                LOCTR = LOCTAB[INDEX_LOCTAB].a[1];
                INDEX_RECORD = 1;
                record = "T";
                INDEX_RECORD += sprintf(buff, "^%06X", LOCTR);
                // record += "^";
                record += buff;
                // record += "^";
                string str = LOCTAB[CURRENT_SECT].tag;
                addSpaces(str, str.size());
                output << "H";
                output << "^";
                output << str;
                output << "^";
                padZeros(output, 6);
                output << LOCTAB[CURRENT_SECT].a[1];
                output << "^";
                padZeros(output, 6);
                output << LOCTAB[CURRENT_SECT].a[2] << "\n";
            }
        }
        else if (OPCODE == "WORD")
        {
            bool isref, kind;
            isref = false;
            kind = false;
            parseExpression(&isref,OPERAND, &kind, &INDEX_LINE); // evaluate the OPERAND
            // parseExpression(OPERAND, &INDEX_LINE, &kind, &isref);
            if (isref)
            {
                MODIF_RECORD.clear();
                string tmp;
                char buff[200];
                MODIF_IND = 0;
                int prev_sign = 1;
                string str = OPERAND;
                bool found = false;
                for (int i = 0; i < str.length(); i++)
                {
                    if (str[i] == '-' || str[i] == '+')
                    {
                        found = false;
                        int k = 0;
                        while (k < SIZE_SYMTAB)
                        {
                            if (SYMTAB[k].tag == tmp && (SYMTAB[k].a[2] != CURRENT_SECT))
                            {
                                MODIF_RECORD = "M";
                                MODIF_IND = sprintf(buff, "^%06X^06", LOCTR);
                                // MODIF_RECORD += "^";
                                MODIF_RECORD += buff;
                                MODIF_IND++;
                                MODIF_RECORD += "^";
                                prev_sign == 0 ? MODIF_RECORD += '-' : MODIF_RECORD += '+';

                                MODIF_IND += SYMTAB[k].tag.length();
                                MODIF_RECORD += SYMTAB[k].tag;
                                modif_util << MODIF_RECORD << "\n";
                            }
                            k++;
                        }
                        tmp.clear();
                        str[i] == '+' ? prev_sign = 1 : prev_sign = -1;
                    }
                    else
                    {
                        tmp += str[i];
                    }
                }
                found = false;
                int k = 0;
                while (k < SIZE_SYMTAB)
                {
                    if (SYMTAB[k].tag == tmp)
                    {
                        if (SYMTAB[k].a[2] != CURRENT_SECT)
                        {
                            MODIF_RECORD = "M";
                            MODIF_IND = sprintf(buff, "^%06X^06", LOCTR) + 1;
                            // MODIF_RECORD += "^";
                            MODIF_RECORD += buff;
                            MODIF_RECORD += "^";
                            prev_sign == -1 ? MODIF_RECORD += '-' : MODIF_RECORD += '+';
                            MODIF_IND += SYMTAB[k].tag.length();
                            MODIF_RECORD += SYMTAB[k].tag;
                            modif_util << MODIF_RECORD << "\n";
                        }
                    }
                    k++;
                }
            }
            OUT_LINE += INDEX_LINE;
            // TEXT_RECORD =;
            sprintf(buff, "^%06X", OUT_LINE);
            // TEXT_RECORD += "^";
            TEXT_RECORD = buff;
        }
        if (OPCODE == "EXTREF")
        { // save entry with respect to EXTREF as R records
            string token = "";
            string refer_record = "R";
            istringstream iss(OPERAND);
            while (getline(iss, token, ','))
            {
                string tmp = token;
                addSpaces(tmp, token.size());
                refer_record += "^";
                refer_record += tmp;
            }
            output << refer_record << "\n";
        }
        else if (OPCODE == "EXTDEF")
        { // save entry with respect to EXTDEF as D records
            string token = "";
            DEF_RECORD = "D";
            DEF_IND = 1;
            istringstream iss(OPERAND);
            while (getline(iss, token, ','))
            {
                findSYMTAB(token);
                if (inSYMTAB)
                {
                    int n = SYMTAB[INDEX_SYMTAB].tag.length();
                    DEF_IND += n;
                    DEF_RECORD += "^";
                    DEF_RECORD += SYMTAB[INDEX_SYMTAB].tag;
                    // DEF_RECORD += "^";
                    DEF_IND += sprintf(buff, "^%06X", SYMTAB[INDEX_SYMTAB].a[0]);
                    DEF_RECORD += buff;
                }
            }
            output << DEF_RECORD << "\n";
        }
        else if (OPCODE == "LTORG" || (TEXT_RECORD.length() + ONE_LINE.length() > 60) || (LOCTR - start_LOCTR) > 27)
        {
            // if current instruction doesn't fit, print current text record line and start a new one
            INDEX_RECORD += sprintf(buff, "^%02X", (int)(ONE_LINE.length() / 2));
            // record += "^";
            record += buff;
            INDEX_RECORD += ONE_LINE.length();
            // record += "^";
            // record += ONE_LINE;
            record += ONE_LINE_HOLDER;
            if (ONE_LINE.length())
            {
                output << record << "\n";
            }
            ONE_LINE_HOLDER.clear();
            ONE_LINE.clear();
            INDEX_RECORD = 1;
            start_LOCTR = LOCTR;
            record = "T";
            // record+="^";
            INDEX_RECORD += sprintf(buff, "^%06X", LOCTR);
            // record += "^";
            record += buff;
        }
        ONE_LINE_HOLDER += TEXT_RECORD;
        ONE_LINE += TEXT_RECORD;
        ONE_LINE.erase(remove(ONE_LINE.begin(), ONE_LINE.end(), '^'), ONE_LINE.end());
        parseLinePass2(intermediate_in);
    }
    while (1)
    {
        TEXT_RECORD.clear();
        parseLinePass2(intermediate_in);
        OUT_LINE = 0;
        if (buffer.length() < 2)
        {
            break;
        }
        if (LITTAB.find(OPCODE) != LITTAB.end())
        {
            OUT_LINE = LITTAB[OPCODE].a[1];
            if ((int)('X' - LITTAB[OPCODE].tag[1]) == 0)
            {
                char temp_ptr[32];
                int temp_index = 0;
                temp_ptr[0] = '\0';
                for (int i = 3; i <= OPCODE.length() - 2;)
                {
                    temp_ptr[temp_index] = OPCODE[i];
                    temp_ptr[temp_index + 1] = '\0';
                    temp_index++;
                    i++;
                }
                sprintf(buff, "^%s", temp_ptr);
                // TEXT_RECORD += "^";
                TEXT_RECORD += buff;
            }
            else
            {
                sprintf(buff, "^%X", OUT_LINE);
                // TEXT_RECORD += "^";
                TEXT_RECORD += buff;
            }
        }
        ONE_LINE_HOLDER += TEXT_RECORD;
        ONE_LINE += TEXT_RECORD; // add object code
        ONE_LINE.erase(remove(ONE_LINE.begin(), ONE_LINE.end(), '^'), ONE_LINE.end());
    }
    INDEX_RECORD += sprintf(buff, "^%02X", (int)ONE_LINE.length() / 2);
    // record += "^";
    record += buff;
    INDEX_RECORD += ONE_LINE.size();
    // record += "^";
    record += ONE_LINE_HOLDER;
    output << record << "\n";
    modif_util.close();
    modif_util.open("modified.txt", ios::in);
    while (getline(modif_util, MODIF_RECORD))
    {
        output << MODIF_RECORD << "\n";
    }
    intermediate_in.close();
    modif_util.close();
    output << "E\n";
    output.close();
    cout<< "Output generated in:\nouput.txt\n";
    return 0;
}