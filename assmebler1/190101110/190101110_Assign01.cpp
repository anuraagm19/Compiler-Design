// CS 348 Programming Assignment 1: 2 PASS ASSEMBLER
// Anuraag Mahajan - 190101110

// Commands to execute program: (LINUX g++)
// $ g++ 190101110_Assign01.cpp
// $ ./a.out <inputfile>

// Output file: out.txt
// Intermediate.txt : Pass1 will create an intermediate file named intermediate.txt containing assigned addresses
// to all statements in the program

#include <bits/stdc++.h>
using namespace std;

/////////////////////////////////////////////////////  ---------- DEFINING ERRORS ------------ ////////////////////////////////////////////
#define E00 "Incomplete instructions,\"END\" directive not found"
#define E10 "Invalid LABEL"
#define E11 "Invalid MNEMONIC"
#define E12 "Invalid OPERAND"
#define E20 "Undefined behaviour"
#define E21 "Memory limit exceeded"

/////////////////////////////////////////////////////  ---------- PARAMETERS ------------ ////////////////////////////////////////////
unordered_set<string> DIRECTIVES = {"START", "END", "BYTE", "WORD", "RESB", "RESW"}; // valid directives

unordered_map<string, string> OPTAB = {
	// valid MNEMONICS with corresponding opcodes
	{"LDA", "00"},
	{"LDX", "04"},
	{"LDL", "08"},
	{"STA", "0C"},
	{"STX", "10"},
	{"STL", "14"},
	{"LDCH", "50"},
	{"STCH", "54"},
	{"ADD", "18"},
	{"SUB", "1C"},
	{"MUL", "20"},
	{"DIV", "24"},
	{"COMP", "28"},
	{"J", "3C"},
	{"JLT", "38"},
	{"JEQ", "30"},
	{"JGT", "34"},
	{"JSUB", "48"},
	{"RSUB", "4C"},
	{"TIX", "2C"},
	{"RD", "D8"},
	{"TD", "E0"},
	{"WD", "DC"},
};

unordered_map<string, int> SYMTAB; // used  to store labels and their corresponding locations

string LABEL, INSTRUCTION, OPERAND, LOCATION, ERROR, LINE; // important parameters
int START_ADDRESS, OPERAND_ADDRESS, RECORD_START, PROGRAM_LENGTH, LOCCTR;
int RAM_SIZE = (1 << 15); // restriction on size of memory
bool commentFlag = 0;

ofstream outfile; // I/O streams
ifstream infile;

/////////////////////////////////////////////////////  ---------- HELPER FUNCTIONS ------------ ////////////////////////////////////////////

void errorHandler()
{
	if (ERROR != E00 && ERROR != E20)
		cout << LINE << "\n";
	cout << "error: " << ERROR << "\n";
	exit(1);
}

void checkHex(string &str) // checks if input string represents hex value
{

	int n = str.length();

	for (int i = 0; i < n; i++)
	{
		char ch = str[i];

		if ((ch < '0' || ch > '9') &&
			(ch < 'A' || ch > 'F')) // only upper-case allowed
		{
			ERROR = E12;
			errorHandler();
		}
	}
}

int getNextLine(ifstream &infile, string &str) // reads new line from input file
{
	if (!getline(infile, str))
		return 0;
	return 1;
}

bool isValidLabel(string &str) // checks if the input label is valid
{
	int len = str.length();
	for (int i = 0; i < len; i++)
	{
		if (!isalnum(str[i])) // should contain only alphanumeric characters
			return 0;
	}
	if (OPTAB.find(str) != OPTAB.end() || DIRECTIVES.find(str) != DIRECTIVES.end()) // label name should not clash with reserved words
		return 0;
	if (isdigit(str[0])) // should not start with a digit
		return 0;
	return 1;
}

string intToHex(int val) // converts decimal to hex string (upppercase)
{
	stringstream stream;
	stream << hex << val;
	string result(stream.str());
	transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}

int parseLinePass1(string &str)
// fetches space separated words from a input file and assigns them to appropriate parameters - LABEL, OPERAND, INSTRUCTION
{

	commentFlag = 0;
	vector<string> args;

	int i = 0;
	int len = str.length();
	for (int i = 0; i < len; i++)
	{
		if (int(str[i]) < 33)
			continue;
		string s;
		while (i < len && int(str[i]) >= 33)
		{
			s += str[i];
			i++;
		}
		args.push_back(s);
	}

	len = args.size();
	if (len == 0)
		return 0;

	if (args[0][0] == '.')
	{
		commentFlag = 1;
		return 1;
	}
	if (len == 3)
	{
		if (isValidLabel(args[0]))
		{
			LABEL = args[0];
			INSTRUCTION = args[1];
			OPERAND = args[2];
		}
		else
		{
			ERROR = E10;
			errorHandler();
		}
		return 1;
	}
	if (len == 2)
	{
		if (isValidLabel(args[0]))
		{
			LABEL = args[0];
			INSTRUCTION = args[1];
			OPERAND = "";
		}
		else
		{
			LABEL = "";
			INSTRUCTION = args[0];
			OPERAND = args[1];
		}
		return 1;
	}
	if (len == 1)
	{
		LABEL = "";
		INSTRUCTION = args[0];
		OPERAND = "";
		return 1;
	}
	return -1;
}

int parseLinePass2(string &str)
// fetches space separated words from intermediate.txt and assigns them to appropriate parameters - LABEL, OPERAND, INSTRUCTION, LOCATION
{
	commentFlag = 0;
	vector<string> args;

	int i = 0;
	int len = str.length();
	for (int i = 0; i < len; i++)
	{
		if (int(str[i]) < 33)
			continue;
		string s;
		while (i < len && int(str[i]) >= 33)
		{
			s += str[i];
			i++;
		}
		args.push_back(s);
	}

	len = args.size();

	if (len == 0)
		return 0;

	if (args[0][0] == '.')
	{
		commentFlag = 1;
		return 1;
	}
	if (len == 4)
	{
		LOCATION = args[0];
		LABEL = args[1];
		INSTRUCTION = args[2];
		OPERAND = args[3];
		return 1;
	}
	if (len == 3)
	{

		if (isValidLabel(args[1]))
		{
			LOCATION = args[0];
			LABEL = args[1];
			INSTRUCTION = args[2];
			OPERAND = "";
		}
		else
		{
			LOCATION = args[0];
			LABEL = "";
			INSTRUCTION = args[1];
			OPERAND = args[2];
		}
		return 1;
	}
	if (len == 2)
	{
		LOCATION = args[0];
		LABEL = "";
		INSTRUCTION = args[1];
		OPERAND = "";
		return 1;
	}
	return -1;
}

void writeLine() // prints output to intermediate file
{
	// if (INSTRUCTION != "END")
	outfile << intToHex(LOCCTR) << "\t\t";
	outfile << LABEL << "\t\t";
	outfile << INSTRUCTION << "\t\t";
	outfile << OPERAND << "\n";
}

int getOperandSize(string &str)
// calculates size indicated by operand for directives, used to calculate next addrress.
{
	int len = str.length();
	if (str[0] == 'C')
	{
		if (INSTRUCTION != "BYTE")
		{
			ERROR = E12;
			errorHandler();
		}
		else if (len < 4)
		{
			ERROR = E12;
			errorHandler();
		}
		else if (str[1] == '\'' && str[len - 1] == '\'')
		{
			if (len - 3 > 30)
			{
				ERROR = E12;
				errorHandler();
			}
			return len - 3;
		}
		else
		{
			ERROR = E12;
			errorHandler();
		}
	}
	else if (str[0] == 'X')
	{
		if (len < 4)
		{
			ERROR = E12;
			errorHandler();
		}
		else if (str[1] == '\'' && str[len - 1] == '\'')
		{
			string hexString;
			int ind = 2;
			while (ind != len - 1)
			{
				hexString += str[ind];
				ind++;
			}
			int num;
			checkHex(hexString);
			try
			{
				num = stoi(hexString, NULL, 16);
			}
			catch (...)
			{
				ERROR = E21;
				errorHandler();
			}

			if (INSTRUCTION == "RESW" || INSTRUCTION == "RESB")
				return num;

			int k = (int)(log2(num));
			k++;

			if (INSTRUCTION == "WORD" && k > 24)
			{
				ERROR = E12;
				errorHandler();
			}
			return (k + 7) / 8;
		}
		else
		{
			ERROR = E12;
			errorHandler();
		}
	}
	else
	{
		for (int i = 0; i < len; i++)
		{
			if (!isdigit(str[i]))
			{
				ERROR = E12;
				errorHandler();
			}
		}
		int num;
		try
		{
			num = stoi(str);
		}
		catch (...)
		{
			ERROR = E21;
			errorHandler();
		}

		if (INSTRUCTION == "RESW" || INSTRUCTION == "RESB")
			return num;

		int k = (int)(log2(num));
		k++;

		if (INSTRUCTION == "WORD" && k > 24)
		{
			ERROR = E12;
			errorHandler();
		}
		return (k + 7) / 8;
	}
	return -1;
}

string adjustLength(string &str, int tar)
// adjusts string length to fit specified format
{
	int len = str.length();
	int diff = tar - len;
	if (diff <= 0)
		return str;
	string prefix;
	while (diff--)
		prefix += '0';
	return (prefix + str);
}

string getOperandValue(string &str)
// extracts value from operand for directives, used to create object code for the instruction
{
	int len = str.length();
	string result;
	if (str[0] == 'C')
	{
		int sum = 0;
		int ind = 2;
		while (ind != len - 1)
		{
			string toAdd = intToHex(str[ind]);
			result += adjustLength(toAdd, 2);
			ind++;
		}
		return result;
	}
	if (str[0] == 'X')
	{

		int ind = 2;
		while (ind != len - 1)
		{
			result += str[ind];
			ind++;
		}
		if (result.length() & 1)
			result = '0' + result;
		return result;
	}
	else
	{
		int num = stoi(str);
		result = intToHex(num);
		if (INSTRUCTION == "BYTE")
		{
			if (result.length() & 1)
				result = '0' + result;
		}
		else
			result = adjustLength(result, 6);
		return result;
	}
}

void printTextRecord(string &str)
// print current text record line to object file
{

	int len = str.length();
	if (len == 0)
		return;
	for (int i = 0; i < len; i++)
		if (str[i] == '^')
			len--;
	len /= 2;
	string head = "T";
	string holder = intToHex(RECORD_START);
	head += "^" + adjustLength(holder, 6);
	holder = intToHex(len);
	holder = adjustLength(holder, 2);
	head += "^" + holder;
	outfile << head << str << "\n";
}

void checkAndParse(string &str, int *res, int pass) // checks the new fetched line before parsing
{
	if (!(*res))
	{
		ERROR = E00;
		errorHandler();
	}
	else
	{
		if (pass == 1)
			*res = parseLinePass1(str);
		else
			*res = parseLinePass2(str);
		if (pass == 1 && *res == -1)
		{
			ERROR = "Too many arguments";
			errorHandler();
		}
		if (pass == 2 && *res == -1)
		{
			ERROR = E20;
			errorHandler();
		}
	}
}

int main(int argc, char **argv)
{

	/////////////////////////////////////////////////////  ---------- PASS 1 ------------ ////////////////////////////////////////////

	// open required I/O streams
	if (argc != 2)
	{
		cout << "Invalid format. Provide input file name as a single argument";
		exit(1);
	}

	string input_file = argv[1];
	infile.open(input_file);
	outfile.open("intermediate.txt");

	// read START
	int res = 0;
	while (commentFlag || res == 0)
	{
		if (commentFlag)
			outfile << LINE << "\n";
		res = getNextLine(infile, LINE);
		checkAndParse(LINE, &res, 1);
	}

	if (INSTRUCTION == "START")
	{
		checkHex(OPERAND);
		try
		{
			LOCCTR = stoi(OPERAND, NULL, 16);
		}
		catch (...)
		{
			ERROR = E21;
			errorHandler();
		}
		if (LOCCTR >= RAM_SIZE)
		{
			ERROR = E21;
			errorHandler();
		}
		writeLine();
		res = getNextLine(infile, LINE);
		checkAndParse(LINE, &res, 1);
	}
	else
	{
		LOCCTR = 0;
	}

	START_ADDRESS = LOCCTR;

	// read further instructions till "END"
	while (INSTRUCTION != "END")
	{
		if (res && !commentFlag)
		{

			if (LABEL.length()) // check and label to SYMTAB with corresponding location counter
			{
				if (SYMTAB.find(LABEL) != SYMTAB.end())
				{
					ERROR = "Duplicate LABEL";
					errorHandler();
				}
				else
					SYMTAB[LABEL] = LOCCTR;
			}

			writeLine(); // write line to intermediate file

			// increment location counter for instructions and directives accordingly
			if (OPTAB.find(INSTRUCTION) != OPTAB.end())
			{
				LOCCTR += 3;
			}
			else if (INSTRUCTION == "WORD") // handle assembler directives
			{
				getOperandSize(OPERAND);
				LOCCTR += 3;
			}

			else if (INSTRUCTION == "RESW")
			{
				LOCCTR += 3 * getOperandSize(OPERAND);
			}
			else if (INSTRUCTION == "RESB")
			{
				LOCCTR += getOperandSize(OPERAND);
			}
			else if (INSTRUCTION == "BYTE")
			{
				LOCCTR += getOperandSize(OPERAND);
			}
			else
			{
				ERROR = E11;
				errorHandler();
			}
			if (LOCCTR >= RAM_SIZE)
			{
				ERROR = E21;
				errorHandler();
			}
		}
		else if (commentFlag) // add comments to intermediate file
		{
			outfile << LINE << "\n";
		}

		// fetch next line
		res = getNextLine(infile, LINE);
		checkAndParse(LINE, &res, 1);
	}

	writeLine();

	PROGRAM_LENGTH = LOCCTR - START_ADDRESS; // set program length

	infile.close();
	outfile.close();

	/////////////////////////////////////////////////////  ---------- PASS 2 ------------ ////////////////////////////////////////////

	// open required I/O streams
	infile.open("intermediate.txt");
	outfile.open("out.txt");

	// start head record
	outfile << "H";
	res = 0;
	while (commentFlag || res == 0)
	{
		res = getNextLine(infile, LINE);
		checkAndParse(LINE, &res, 2);
	}

	if (INSTRUCTION == "START")
	{
		if (LABEL.length())
		{
			if (LABEL.length() <= 6)
			{
				while (LABEL.length() != 6)
					LABEL += " ";
			}
			else
			{
				string newLABEL = "";
				int ind = 0;
				while (newLABEL.length() != 6)
				{
					newLABEL += LABEL[ind];
					ind++;
				}
				LABEL = newLABEL;
			}
			outfile << "^" << LABEL;
		}
		else
		{
			outfile << "^"
					<< "PROG  ";
		}

		res = getNextLine(infile, LINE);
		checkAndParse(LINE, &res, 2);
	}

	string holder = intToHex(START_ADDRESS);
	outfile << "^" << adjustLength(holder, 6);

	holder = intToHex(PROGRAM_LENGTH);
	outfile << "^" << adjustLength(holder, 6) << "\n";

	RECORD_START = START_ADDRESS;

	// start text records

	string TextRecord;
	int limit = 60;
	int extraChars = 0;

	while (INSTRUCTION != "END")
	{
		if (!commentFlag && res)
		{
			if (OPTAB.find(INSTRUCTION) != OPTAB.end())
			{
				bool indexAddressing = 0;
				if (OPERAND.length())
				{
					int len = OPERAND.length();
					if (len > 2 && OPERAND[len - 1] == 'X' && OPERAND[len - 2] == ',') // for index based addressing mode
					{
						string newOPERAND = "";
						for (int i = 0; i < len - 2; i++)
							newOPERAND += OPERAND[i];
						OPERAND = newOPERAND;
						indexAddressing = 1;
					}
					if (SYMTAB.find(OPERAND) != SYMTAB.end())
					{
						OPERAND_ADDRESS = SYMTAB[OPERAND];
					}
					else
					{
						ERROR = E12;
						errorHandler();
					}
				}
				else
				{
					OPERAND_ADDRESS = 0;
				}

				string head = OPTAB[INSTRUCTION];

				if (indexAddressing) // set bit for index based addressing
				{
					OPERAND_ADDRESS += (1 << 15);
				}

				holder = intToHex(OPERAND_ADDRESS);
				string tail = adjustLength(holder, 4);

				if (limit - 6 < 0) // if current instruction doesn't fit, print current text record line and start a new one
				{
					printTextRecord(TextRecord);
					TextRecord = "^" + head + tail;
					extraChars = 1;
					RECORD_START = stoi(LOCATION, NULL, 16);
				}
				else // add current instruction to text record
				{
					if (TextRecord.length() == 0)
						RECORD_START = stoi(LOCATION, NULL, 16);
					TextRecord += "^" + head + tail;
					extraChars++;
				}
			}
			else if (INSTRUCTION == "BYTE" || INSTRUCTION == "WORD") // get object code for BYTE and WORD using operand value
			{
				string add = getOperandValue(OPERAND);
				int len = add.length();
				if (limit - len < 0)
				{
					printTextRecord(TextRecord);
					TextRecord = "^" + add;
					extraChars = 1;
					RECORD_START = stoi(LOCATION, NULL, 16);
				}
				else
				{
					if (TextRecord.length() == 0)
						RECORD_START = stoi(LOCATION, NULL, 16);
					TextRecord += "^" + add;
					extraChars++;
				}
			}
			else // end text record for RESB and RESW
			{
				printTextRecord(TextRecord);
				TextRecord = "";
				extraChars = 0;
			}
			limit = 60 - TextRecord.length() + extraChars;
		}
		// fetch next line
		res = getNextLine(infile, LINE);
		checkAndParse(LINE, &res, 2);
	}
	printTextRecord(TextRecord);

	// print end record
	outfile << "E";
	if (SYMTAB.find(OPERAND) == SYMTAB.end())
	{
		ERROR = E12;
		errorHandler();
	}
	int exec_address = SYMTAB[OPERAND];
	holder = intToHex(exec_address);
	outfile << "^" << adjustLength(holder, 6);
	return 0;
}
