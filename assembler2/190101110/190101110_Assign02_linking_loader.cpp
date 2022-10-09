/*

CS348 Programming  Assignmment 2
Anuraag Mahajan 1901011110

 Commands to execute program: (LINUX g++)
 $ g++ 190101110_Assign02_linking_loader.cpp
 $ ./a.out <inputfile>

 Output file:
 estab.txt - pass1
 memory_map.txt - pass2
 memory_listing.txt - extra
*/

#include <bits/stdc++.h>
using namespace std;

/////////////////////////////////////////////////////  ---------- PARAMETERS ------------ ////////////////////////////////////////////
string CURR_CS;			// name of the current control section
int CS_ADDR, PROG_ADDR; // control_section_address , program_address
string st_adr = "0000"; // starting address assumend at 0

string line;

// I/O streams
ifstream input;
ofstream memory_map;
ofstream estab;
ofstream objectfile; // extra

map<int, string> addr_map;
unordered_map<string, pair<int, int>> ESTAB; // ext symbol table


/////////////////////////////////////////////////////  ---------- HELPER FUNCTIONS ------------ ////////////////////////////////////////////


// fetches space separated words from input line
void lineParser(string &str, vector<string> &args)
{

	str.erase(remove(str.begin(), str.end(), '^'), str.end());
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
}


// interger to hexadecimal string
string to_hex_string(int val, int pad)
{
	stringstream stream;
	stream << hex << val;
	string result(stream.str());
	while (result.length() < pad)
		result = '0' + result;
	transform(result.begin(), result.end(), result.begin(), ::toupper);
	return result;
}

// convert hex string to decimal int
int hex_to_int(string &hexa)
{
	char *p;
	return strtol(hexa.c_str(), &p, 16);
}

int main(int argc, char **argv)
{
	// open required I/O streams
	if (argc != 2)
	{
		cout << "Provide input file name as a single argument\n";
		exit(1);
	}
	string inputfile = argv[1];
	input.open(inputfile);
	if (!input.is_open())
	{
		printf("Cannot open input file\n");
		exit(1);
	}
	/////////////////////////////////////////////////////  ---------- PASS 1 ------------ ////////////////////////////////////////////

	// pass 1:  handle H and D records
	ESTAB.clear();
	PROG_ADDR = hex_to_int(st_adr);


	estab.open("estab.txt");

	estab << "C-section\tSymbol Name  Address \t Length\n";

	CS_ADDR = PROG_ADDR; // assign starting address for relocation
	line.clear();

	while (getline(input, line, '\n'))
	{
		vector<string> args;
		lineParser(line, args);
		if (args.size() == 0)
			continue;
		if (args[0][0] == 'H') //  new control section
		{
			string cs_len = "", CS_ADDR_obj = ""; // reading the address and length from the next word
			for (int i = 0; i < 6; i++)
			{
				CS_ADDR_obj += args[1][i];
				cs_len += args[1][6 + i];
			}

			// get name of the control section
			args[0] = args[0].substr(1);
			if (ESTAB.find(args[0]) != ESTAB.end())
			{
				cout << "ERROR: Duplicate section\n";
			}
			else
			{

				CURR_CS = args[0];
				// insert the control section in ESTAB;
				pair<int, int> new_es;
				new_es.second = hex_to_int(CS_ADDR_obj) + CS_ADDR;
				new_es.first = hex_to_int(cs_len);
				ESTAB[args[0]] = new_es;
				// print 
				estab << args[0] << " \t\t  \t\t\t " << to_hex_string(new_es.second, 4) << " \t\t " << to_hex_string(new_es.first, 4) << " \t\t \n";
			}
			while (getline(input, line, '\n')) // reading till E is found
			{
				vector<string> records;
				lineParser(line, records);
				if (records[0][0] == 'D') // define records
				{

					records[0] = records[0].substr(1); // remove D 
					for (int i = 1; i < records.size(); i++)
						records[0].append(records[i]);

					// cout << records[0] << "*\n";
					// string es_name = records[0];
					int i = 0;
					while (i < records[0].size())
					{
						string es_name, es_addr;
						while (i < records[0].size() && records[0][i] != '0')
						{
							es_name += records[0][i];
							// cout<<es_name;
							i++;
						}
						if (i < records[0].size())
						{
							for (int j = i; j < i + 6; j++)
								es_addr += records[0][j];
							i += 6;
						}

						// cout<<es_name<<" "<<es_addr<<"\n";
						if (ESTAB.find(es_name) != ESTAB.end())
						{
							cout << "ERROR : duplicate external symbol\n";
						}
						else
						{
							// insert the new external symbol in the ESTAB
							pair<int, int> new_es;
							new_es.first = 0;
							new_es.second = hex_to_int(es_addr) + CS_ADDR;
							ESTAB[es_name] = new_es;

							// print it to the ESTAB file
							estab << "    \t\t" << es_name << " \t\t " << to_hex_string(ESTAB[es_name].second, 4) << " \t\t \n";
						}
					}
				}
				if (records[0][0] == 'E')
				{
					break;
				}
			}
			CS_ADDR += hex_to_int(cs_len); // for starting address for next section
		}
	}
	estab.close();
	input.close();

	/////////////////////////////////////////////////////  ---------- PASS 2 ------------ ////////////////////////////////////////////


	input.open(inputfile);
	CS_ADDR = PROG_ADDR;
	int cs_len = 0;

	while (getline(input, line, '\n'))
	{
		vector<string> args;
		lineParser(line, args);
		if (args.size() == 0)
			continue;

		if (args[0][0] == 'H') // new control section
		{
			// args[0].erase(args[0].begin());
			args[0] = args[0].substr(1);
			CURR_CS = args[0];
			cs_len = ESTAB[CURR_CS].first;

			while (getline(input, line, '\n'))
			{
				vector<string> records;
				lineParser(line, records);
				if (records[0][0] == 'T') // text record
				{
					string specified_addr = "";
					for (int i = 1; i < 7; i++)
					{
						specified_addr += records[0][i];
					}
					int location = CS_ADDR + hex_to_int(specified_addr);

					// put the record in the memory location calculated
					for (int i = 9; i < records[0].length(); i += 2)
					{
						addr_map[location] = records[0][i];
						addr_map[location] += records[0][i + 1];
						location++;
					}
				}
				else if (records[0][0] == 'M') // modification record
				{

					string sym_name, sym_loc, sym_len;

					for (int j = 1; j < records[0].length(); j++)
					{
						if (j < 7)
							sym_loc += records[0][j];
						else if (j < 10)
							sym_len += records[0][j];
						else
							sym_name += records[0][j];
					}

					sym_name.erase(remove(sym_name.begin(), sym_name.end(), ' '), sym_name.end());
					// remove spaces if any

					int z = hex_to_int(sym_loc) + CS_ADDR;

					// reading old value from memory
					string og = "";
					og += addr_map[z] + addr_map[z + 1] + addr_map[z + 2];

					int new_val = 0;

					if (og[0] == 'F') // handle negative values of the memory
					{
						long int BORR = (long int)(0xFFFFFFFF000000);
						new_val = BORR;
					}

					// add or subtract as per + or - in modification records
					new_val += hex_to_int(og);
					sym_len.back() == '+' ? new_val += ESTAB[sym_name].second : new_val -= ESTAB[sym_name].second;

					string hexa = to_hex_string(new_val, 6);

					if (hexa.length() > 6 && hexa[0] == 'F' && hexa[1] == 'F')
					{
						hexa = hexa.substr(2);
					}

					// store new value in the memory
					addr_map[z] = addr_map[z + 1] = addr_map[z + 2] = "";
					addr_map[z + 2] += hexa[4];
					addr_map[z + 2] += hexa[5];
					addr_map[z + 1] += hexa[2];
					addr_map[z + 1] += hexa[3];
					addr_map[z] += hexa[0];
					addr_map[z] += hexa[1];
				}
				else if (records[0][0] == 'E') // end of section
				{
					break;
				}
			}
			if (line[0] == 'E' && line.length() > 1)
			{
				line = line.substr(1);
			}
			CS_ADDR += cs_len;
		}
	}
	input.close();

	/////////////////////////////////// PRINTING LOADER INSTRUCTIONS /////////////////////////////////
	objectfile.open("memory_listing.txt");
	objectfile << "Address\tMemory_Content\n";
	for (auto it = addr_map.begin(); it != addr_map.end(); it++)
		objectfile << to_hex_string(it->first, 4) << "\t" << it->second << "\n";
	objectfile.close();

	/////////////////////////////////// PRINTING MEMORY MAP /////////////////////////////////

	memory_map.open("memory_map.txt");

	int i = 0;
	int end_addre = addr_map.rbegin()->first;

	int curr_addr = PROG_ADDR;
	curr_addr -= 16;

	while (1)
	{
		if (end_addre < curr_addr)
		{
			while ((PROG_ADDR - curr_addr) % 16 != 0)
			{
				(i + 1) % 4 == 0 ? memory_map << "xx\t" : memory_map << "xx";
				curr_addr++;
				i++;
			}
			break;
		}
		if (i % 16 == 0)
		{
			memory_map << to_hex_string(curr_addr, 4) << " \t ";
		}

		if (curr_addr < PROG_ADDR)
		{
			(i + 1) % 4 == 0 ? memory_map << "xx\t" : memory_map << "xx";
		}
		else if (addr_map.find(curr_addr) != addr_map.end() && addr_map[curr_addr].length())
		{
			(i + 1) % 4 == 0 ? memory_map << addr_map[curr_addr] << " \t" : memory_map << addr_map[curr_addr];
		}
		else
		{
			(i + 1) % 4 == 0 ? memory_map << "..\t" : memory_map << "..";
		}

		if ((i + 1) % 16 == 0)
			memory_map << "\n";
		i++;
		curr_addr++;
	}
	memory_map.close();
	cout<< "Output generated in:\nmemory_map.txt\nestab.txt\nmemory_lising.txt\n";
}