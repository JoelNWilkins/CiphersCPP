#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <getopt.h>
using namespace std;

const char *help_message = "Usage:\n"
    "  vigenere [options] [input_text]\n\n"
    "Options:\n"
    "  -h, --help           Show this help message.\n"
    "  -v, --version        Show the version.\n"
    "  -i, --input=<file>   The file to apply the cipher to.\n"
    "  -o, --output=<file>  The file to save the output to.\n"
    "  -k, --key=<value>    The key to use for the cipher [default: 3].\n"
    "                       Letters or comma separated integers may be used as keys.\n"
    "                       The Vigenere cipher with a key of length 1 is just a shift cipher.\n"
    "  -d, --decode         Apply the inverse key to decode the cipher.\n"
    "  -u, --upper          Convert the output to upper case.\n"
    "  -l, --lower          Convert the output to lower case.\n"
    "  -b, --block=<value>  The number of letters to group together in a block [default: 0].\n"
    "                       If the value is 0, then no grouping will occur.\n"
    "                       If the value is -1, then all spaces will be removed.\n"
    "  -c, --cols=<value>   The maximum number of columns [default: -1].\n";

enum CASE { DEFAULTCASE = -1, LOWERCASE = 0, UPPERCASE = 1 };

string change_case(string text, int case_flag) {
    if (case_flag == UPPERCASE) {
        for (auto& x : text) {
            x = toupper(x);
        }
    } else if (case_flag == LOWERCASE) {
        for (auto& x : text) {
            x = tolower(x);
        }
    }
    return text;
}

string break_lines(string text, int cols) {
    string output, word;
    char sep;
    int i = 0;
    text += " ";
    for (auto& x : text) {
        // ASCII values: 10 line break, 32 space, 45 hyphen, 47 forward slash
        if (x == 10 || x == 32 || x == 45 || x == 47) {
            int n = word.length();
            if (i == 0) {
                // This is the first word, so we don't want a space before it
                output += word;
                i += n;
            } else if (i + n < cols) {
                // This word fits on the current line, so we don't need to start a new line
                output += sep + word;
                i += n + 1;
            } else if (sep == 32) {
                // This word won't fit on the current line, so we should start a new line
                output += "\n" + word;
                i = n;
            } else {
                // Put the separating character on the same line if possible,
                // if not put it at the start of the next line
                if (i < cols) {
                    string prefix{sep, '\n'};
                    output += prefix + word;
                    i = n;
                } else {
                    string prefix{'\n', sep};
                    output += prefix + word;
                    i = sep == 10 ? n : n + 1;
                }
            }
            word = "";
            sep = x;
        } else {
            word += x;
        }
    }
    return output + "\n";
}

string format_text(string text, int case_flag, int block_size, int cols, int start) {
    text = change_case(text, case_flag);
    if (block_size == 0) {
        return break_lines(text, cols);
    }
    int blocks_per_row = block_size > 0 ? (cols + 1) / (block_size + 1) : 1;
    if (block_size < 0) {
        block_size = cols;
    }
    string output;
    int i = start;
    for (auto& x : text) {
        // Uppercase letters from 65 to 90
        // Lowercase letters from 97 to 122
        if ((65 <= x && x <= 90) || (97 <= x && x <= 122)) {
            output += x;
            i++;

            if (block_size > 0 && i % block_size == 0) {
                if ((i / block_size) % blocks_per_row == 0) {
                    output += "\n";
                } else {
                    output += " ";
                }
            }
        }
    }
    return output;
}

vector<int> parse_key(string keystring, bool decode) {
    vector<int> key;
    int sign = decode ? -1 : 1;
    stringstream string_stream(keystring);
    string substring;
    while (getline(string_stream, substring, ',')) {
        try {
            // Attempt to convert the substring to an int
            key.push_back(stoi(substring) * sign);
        } catch (invalid_argument) {
            // If the substring cannot be converted to an int then look for letters
            for (auto& x : substring) {
                // Uppercase letters from 65 to 90
                // Lowercase letters from 97 to 122
                if (65 <= x && x <= 90) {
                    key.push_back((x - 65) * sign);
                } else if (97 <= x && x <= 122) {
                    key.push_back((x - 97) * sign);
                }
            }
        }
    }
    return key;
}

string vigenere(string text, vector<int> key, int &i) {
    int n = key.size();
    for (auto& x : text) {
        // Uppercase letters from 65 to 90
        // Lowercase letters from 97 to 122
        // Use (a % b + b) % b instead of just a % b as the latter could be
        // negative for negative values of a (depending on the implementation?)
        if (65 <= x && x <= 90) {
            x = 65 + ((x - 65 + key.at(i % n)) % 26 + 26) % 26;
            i++;
        } else if (97 <= x && x <= 122) {
            x = 97 + ((x - 97 + key.at(i % n)) % 26 + 26) % 26;
            i++;
        }
    }
    return text;
}

int main(int argc, char *argv[]) {
    ifstream InputFile;
    ofstream OutputFile;
    string keystring;
    bool decode = false;
    int case_flag = DEFAULTCASE;
    int block_size;
    int cols = -1;

    int opt;
    struct option long_options[] = {
        {"help", no_argument, nullptr, 'h'},
        {"version", no_argument, nullptr, 'v'},
        {"input", required_argument, nullptr, 'i'},
        {"output", required_argument, nullptr, 'o'},
        {"key", required_argument, nullptr, 'k'},
        {"decode", no_argument, nullptr, 'd'},
        {"upper", no_argument, nullptr, 'u'},
        {"lower", no_argument, nullptr, 'l'},
        {"block", required_argument, nullptr, 'b'},
        {"cols", required_argument, nullptr, 'c'},
        {nullptr, 0, nullptr, 0}
    };

    while (true) {
        int option_index = 0;
        opt = getopt_long(argc, argv, "hvi:o:k:dulb:c:", long_options, &option_index);
        if (opt == -1) break;

        switch (opt) {
            case 'h':
                cout << help_message << endl;
                return 0;
            case 'v':
                cout << "Version 1.0" << endl;
                return 0;
            case 'i':
                InputFile.open(optarg);
                break;
            case 'o':
                OutputFile.open(optarg);
                break;
            case 'k':
                keystring = optarg;
                break;
            case 'd':
                decode = true;
                break;
            case 'u':
                case_flag = UPPERCASE;
                break;
            case 'l':
                case_flag = LOWERCASE;
                break;
            case 'b':
                try {
                    block_size = stoi(optarg);
                } catch (invalid_argument) {
                    cerr << "Invalid block size: " << optarg << endl;
                    return 1;
                }
                break;
            case 'c':
                try {
                    cols = stoi(optarg);
                } catch (invalid_argument) {
                    cerr << "Invalid number of columns: " << optarg << endl;
                    return 1;
                }
                break;
            default:
                return 1;
        }
    }

    // If a key has not been specified accept input from the console
    if (keystring.length() == 0) {
        cout << "Key: ";
        getline(cin, keystring);
    }

    vector<int> key = parse_key(keystring, decode);

    // In each case, if an output file has not been provided, then output to the console
    string line;
    int start = 0;
    if (InputFile.is_open()) {
        // Apply the cipher to the text in the input file provided
        // This takes priority over the [input_text] argument, if it is present
        while (getline(InputFile.is_open() ? InputFile : cin, line)) {
            line = format_text(line, case_flag, block_size, cols, start);
            line = vigenere(line, key, start);
            (OutputFile.is_open() ? OutputFile : cout) << line;
        }
    } else if (optind < argc) {
        // If the [input_text] argument has been provided, then use it as the input
        line = format_text(argv[optind], case_flag, block_size, cols, start);
        line = vigenere(line, key, start);
        (OutputFile.is_open() ? OutputFile : cout) << line;
    } else {
        // If no other input has been specified, then accept input from the console
        cout << " In: ";
        getline(cin, line);
        line = format_text(line, case_flag, block_size, cols, start);
        line = vigenere(line, key, start);
        (OutputFile.is_open() ? OutputFile : cout << "Out: ") << line;
    }

    // Close the files
    InputFile.close();
    OutputFile.close();
    return 0;
}