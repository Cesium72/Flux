#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

string outp = "";
vector<string> subNames;
vector<vector<string>> subVals;
vector<string> typeNames;
vector<unsigned long int> typeSizes;
vector<string> varNames;
vector<string> varTypes;
vector<long unsigned> varAddrs;
vector<string> aliasNames;
vector<string> aliasVals;
unsigned long int nextAddr = 11;

string file_name_arg;
bool dont_optimize_bf_arg = false; // --no_optimize, -n
bool no_warnings_arg = false; // --no_warning, -w
bool help_arg; // --help, -h
bool verbose_arg; // --verbose, -v
bool output_file; // --output, -o

void help_menu(int argc, char* argv[]) {
    cout << "Usage:" << endl <<
            argv[0] << " <input_file> [OPTIONS]" << endl <<
            "Description: A compiler for the Flux language into BF." << endl <<
            endl <<
            "Options:" << endl <<
            "   --help, -h              Displays help menu" << endl <<
            "   --verbose, -v           Outputs verbose" << endl <<
            "   --output, -o <file>     Sets output file" << endl <<
            "   --no_warning, -w        Doesn't display BF warnings - NOT RECOMENDED" << endl <<
            "   --no_optimize, -n       Doesn't optimize BF by removing useless character sequences - NOT RECOMENDED" << endl;
    exit(2);
}

void parse_args(int argc, char* argv[]) {
    int i = 1;
    bool input_file_found;
    while (i <= (argc - 1)) {
        if (argv[i] == "--no_optimize" || argv[i] == "-n") {
            dont_optimize_bf_arg = true;
        } else if (argv[i] == "--no_warning" || argv[i] == "-w") {
            no_warnings_arg = true;
        } else if (argv[i] == "--help" || argv[i] == "-h") {
            help_menu(argc, argv);
        } else if (argv[i] == "--verbose" || argv[i] == "-v") {
            verbose_arg = true;
        } else if (argv[i] == "--output" || argv[i] == "-o") {
            i ++;
            if (i < (argc - 1)) {
                output_file = argv[i];
            } else {
                cerr << "Value not found for " << argv[i-1] << endl;
                help_menu(argc, argv); 
            }
        } else {
            if (!input_file_found) {
                file_name_arg = argv[i];
                input_file_found = true;
            } else {
                cerr << "To many arguments." << endl;
                help_menu(argc, argv);
            }
        }

        i++;
    }

    if (!input_file_found) {
        cerr << "No input file specified." << endl;
        help_menu(argc, argv);
    }
}

typedef struct {
    uint64_t line;
    string prgm_name;
} ProgPos;



void error(string msg, ProgPos pos) {
    cerr << "Error in line " << pos.line << " of " << pos.prgm_name << endl;
    cerr << msg << endl;
    exit(1);
}

string optimize_bf(string data) {
    string output;
    output = data;
    return output;
}

string reduceType(string alias, ProgPos pos) {
    if(find(aliasNames.begin(), aliasNames.end(), alias) != aliasNames.end()) {
        string val = aliasVals[distance(find(aliasNames.begin(), aliasNames.end(), alias), aliasNames.begin())];
        if(find(typeNames.begin(), typeNames.end(), val) == typeNames.end()) {
            error("Could not reduce type \"" + alias + "\"\nPerhaps you have an alias that refers to an alias?", pos);
        }
        return val;
    } else if(find(typeNames.begin(), typeNames.end(), alias) != typeNames.end()) {
        return alias;
    } else {
        error("Invalid type or alias \"" + alias + "\"", pos);
    }
}

void compile(vector<string> program, string program_name) {

    ProgPos pos;
    pos.prgm_name = program_name;

    for(uint64_t i = 0; i < program.size(); i++) {
        pos.line = i+1;
        vector<string> chunks = {""};
        program[i].erase(program[i].find_last_not_of(" \n\r\t") + 1);
        for(int j = 0; j < program[i].size(); j++) {
            if(program[i][j] == ' ') {
                chunks.push_back("");
            } else {
                chunks[chunks.size() - 1] += program[i][j];
            }
        }
        if (chunks[0] == "//" || chunks[0] == "") {
            
        } else if (chunks[0] == "inc") {
            outp += '+';
        } else if (chunks[0] == "dec") {
            outp += '-';
        } else if (chunks[0] == "fwd") {
            outp += ">";
        } else if (chunks[0] == "bck") {
            outp += "<";
        } else if (chunks[0] == "opn") {
            outp += "[";
        } else if (chunks[0] == "end") {
            outp += "]";
        } else if (chunks[0] == "out") {
            outp += ".";
        } else if (chunks[0] == "read") {
            outp += ",";
        } else if (chunks[0] == "type") {
            typeNames.push_back(chunks[1]);
            typeSizes.push_back(stoul(chunks[2]));
        } else if (chunks[0] == "new") {
            if(find(typeNames.begin(), typeNames.end(), chunks[1]) == typeNames.end()) {
                cout << "Attempted to use the type \"" << chunks[1] << "\" before it was declared at line " << i + 1 << " of main" << endl;
                exit(0);
            }
            varTypes.push_back(chunks[1]);
            varNames.push_back(chunks[2]);
            varAddrs.push_back(nextAddr);
            nextAddr += typeSizes[distance(find(typeNames.begin(), typeNames.end(), chunks[1]), typeNames.begin())];
        } else if (chunks[0] == "rep") {
            outp += string(stoul(chunks[2]), program[i][4]);
        } else if (chunks[0] == "macro") {
            subNames.push_back(program[i].substr(6));
            subVals.push_back({});
            while(program[i] != "/macro") {
                i++;
                if(program[i].size() >= 6 && program[i].substr(0, 6) == "macro ") {
                    cout << "Attempted to define a macro inside a macro." << endl;
                    exit(0);
                } else {
                    subVals[subVals.size() - 1].push_back(program[i]);
                }

            }
        } else if (chunks[0] == "to" || chunks[0] == "from") {
            if(find(varNames.begin(), varNames.end(), chunks[1]) != varNames.end()) {
                outp += string(varAddrs[distance(find(varNames.begin(), varNames.end(), chunks[1]), varNames.begin())] - 1, chunks[0] == "to" ? '>': '<');
            } else {
                cout << "Attempted to use the variable \"" << chunks[1] << "\" before it  was declared at line " << i + 1 << " of main" << endl;
                exit(0);
            }
        } else if (chunks[0] == "alias") {
            // typeNames.push_back(chunks[1]);

            // chunks[2]

            // typeSizes.push_back(stoul());
        } else if (chunks[0] == "import") {
            if (chunks[1] != program_name) {
                vector<string> read_code;
                ifstream iFile(chunks[1]);
                if(iFile.is_open()) {
                    string line;
                    while(getline(iFile, line)) {
                        read_code.push_back(line);
                    }
                    compile(read_code,chunks[1]);

                } else {
                    error("Fatal error: Could not import file: \n", pos);
                }
            }
        } else {
            error("Invalid operation \"" + chunks[0] + "\"", pos);
        }
    }
}

int main(int argc, char* argv[]) {
    parse_args(argc, argv);
    
    string fname;
    
    if (file_name_arg.size() > 0) {
        fname = file_name_arg;
        vector<string> code;
        ifstream iFile(fname);
        if(iFile.is_open()) {
            string line;
            while(getline(iFile, line)) {
                code.push_back(line);
            }
            compile(code, fname);
            cout << outp << endl;
        } else {
            cerr << "Fatal error: Could not open file\n";
            return -1;
        }
        return 0;
    } else {
        help_menu(argc, argv);
    }
}