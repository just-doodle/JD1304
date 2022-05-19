#include "jdasm.h"

#define RED "\x1B[31m"
#define YEL "\x1B[33m"
#define CYN "\x1B[36m"
#define RESET "\x1B[0m"

#define PRINTE(...) printf(RED, __VA_ARGS__, RESET);

using namespace std;

vector<string> split(const char *str, char c = ' ')
{
    vector<string> result;

    do
    {
        const char *begin = str;

        while(*str != c && *str)
            str++;

        result.push_back(string(begin, str));
    } while (0 != *str++);

    return result;
}

void assemble(const char *filename, const char *output)
{
    ifstream in(filename, ios::binary);
    ofstream out(output, ios::binary);

    if (!in.is_open())
    {
        cerr << "Could not open file " << filename << endl;
        exit(1);
    }

    if (!out.is_open())
    {
        cerr << "Could not open file " << output << endl;
        exit(1);
    }

    Instruction ins;

    uint32_t filesize = 0;
    in.seekg(ios::end);
    filesize = in.tellg();
    in.seekg(ios::beg);

    vector<string> tokens;
    map<string, uint16_t> labels;
    map<string, string> definitions;

    int isDefined = 0;
    int isNDefined = 0;
    int isMCMP = 0;

    string line;
    uint8_t buffer[1024 * 64];
    memset(buffer, 0, 1024 * 64);
    uint16_t addr = 0;
    int DD_JMP = 0;
    int linenum = 0;

asmloop:
    while (getline(in, line))
    {
        linenum++;
        tokens = split(line.c_str());
        if (tokens.size() == 0)
            continue;

        if(isDefined == 1 || isNDefined == 1 || isMCMP == 1)
        {
            char* c = new char[tokens[0].size()];
            memset(c, 0, tokens[0].size());
            for(int i = 1; i < tokens[0].size(); i++)
            {
                c[i - 1] = tokens[0][i];
            }

            if(strcmp(c, "ENDIF") == 0)
            {
                if(isDefined == 1)
                    isDefined = 2;
                else if(isNDefined == 1)
                    isNDefined = 2;
                else if(isMCMP == 1)
                    isMCMP = 2;
            }
            else
            {
                goto asmloop;
            }
        }
        
        if(tokens[0] == "MEMORY_BEGIN:")
        {
            if(tokens[1][0] == '#')
            {
                char* addr_c = new char[tokens[1].size()];
                for(int i = 1; tokens[1][i] != 'h' && i < tokens[1].size(); i++)
                {
                    addr_c[i - 1] = tokens[1][i];
                }
                addr = (uint16_t)strtol(addr_c, NULL, 16);
            }
            else
            {
                addr = (uint16_t)strtol(tokens[1].c_str(), NULL, 10);
            }
            printf("MEMORY_BEGIN: %x\n", addr);
        }
        else if(tokens[0] == "LDA")
        {
            if(tokens[1][0] == '#') // ABSOLUTE
            {
                char* addr_c = new char[tokens[1].size()];
                for(int i = 1; tokens[1][i] != 'h' && i < tokens[1].size(); i++)
                {
                    addr_c[i - 1] = tokens[1][i];
                }
                buffer[addr++] = ins.CHAM;
                buffer[addr++] = AM_ABS;
                buffer[addr++] = ins.LDA;
                buffer[addr++] = (uint8_t)strtol(addr_c, NULL, 16);
            }
            else if(tokens[1][0] == '$') // IMMEDIATE
            {
                char* addr_c = new char[tokens[1].size()];
                bool isHex = false;
                for(int i = 1; i < tokens[1].size(); i++)
                {
                    if(tokens[1][i] == 'h')
                    {
                        isHex = true;
                        break;
                    }
                    addr_c[i - 1] = tokens[1][i];
                }
                buffer[addr++] = ins.CHAM;
                buffer[addr++] = AM_IMM;
                buffer[addr++] = ins.LDA;
                if(isHex)
                {
                    buffer[addr++] = (uint8_t)strtol(addr_c, NULL, 16);
                }
                else
                {
                    buffer[addr++] = (uint8_t)strtol(addr_c, NULL, 10);
                }
            }
            else if(tokens[1][0] == '\'') // IMMEDIATE (CHAR)
            {
                if(tokens[1][2] == '\'')
                {
                    char a_c = tokens[1][1];
                    buffer[addr++] = ins.CHAM;
                    buffer[addr++] = AM_IMM;
                    buffer[addr++] = ins.LDA;
                    buffer[addr++] = a_c;
                }
                else
                {
                    printf("Expected '\'' after LDA\n");
                    exit(1);
                }
            }
            else if(tokens[1][0] == '[') // INDIRECT
            {
                char* addr_c = new char[tokens[1].size()];
                for(int i = 1; tokens[1][i] != ']'; i++)
                {
                    if(i > tokens[1].size())
                    {
                        printf("Expected ']' after LDA\n");
                        exit(1);
                    }
                    addr_c[i - 1] = tokens[1][i];
                }
                buffer[addr++] = ins.CHAM;
                buffer[addr++] = AM_IND;
                buffer[addr++] = ins.LDA;
                if(addr_c[0] == '%') // Indirect Label
                {
                    char* label_c = new char[tokens[1].size()];
                    for(int i = 1; addr_c[i] != '\0'; i++)
                    {
                        label_c[i - 1] = addr_c[i];
                    }
                    buffer[addr++] = labels[label_c] + DD_JMP;
                    buffer[addr++] = (labels[label_c] + DD_JMP) >> 8;
                    printf("LDA [%%%s] --> 0x%04x\n", label_c, labels[label_c] + DD_JMP);
                }
                else
                {
                    buffer[addr++] = (uint8_t)strtol(addr_c, NULL, 16);
                    buffer[addr++] = (uint8_t)strtol(addr_c, NULL, 16) >> 8;
                }
            }
            else if(tokens[1][0] == '%') // LABEL
            {
                char* label_c = new char[tokens[1].size()];
                for(int i = 1; tokens[1][i] != '%'; i++)
                {
                    if(i > tokens[1].size())
                    {
                        printf("Expected '%%' after LDA\n");
                        exit(1);
                    }
                    label_c[i - 1] = tokens[1][i];
                }
                buffer[addr++] = ins.CHAM;
                buffer[addr++] = AM_ABS;
                buffer[addr++] = ins.LDA;
                buffer[addr++] = labels[label_c] + 3;
                buffer[addr++] = (labels[label_c] + 3) >> 8;
                printf("OPERAND: %s->%x\n", label_c, buffer[labels[label_c] + 3]);
            }
            else
            {
                printf("Expected ABSOLUTE, IMMEDIATE, INDIRECT or CHAR after LDA\n");
                exit(1);
            }
        }
        else if(tokens[0] == "DD")
        {
            if(tokens[1][0] == '#') // HEX
            {
                char* addr_c = new char[tokens[1].size()];
                for(int i = 1; tokens[1][i] != 'h' && i < tokens[1].size(); i++)
                {
                    addr_c[i - 1] = tokens[1][i];
                }
                uint16_t data = (uint16_t)strtol(addr_c, NULL, 16);
                if(data < 0xFF)
                {
                    DD_JMP = 1;
                    buffer[addr++] = ins.INP;
                    buffer[addr++] = data;
                }
                else
                {
                    DD_JMP = 1;
                    buffer[addr++] = ins.AP2;
                    buffer[addr++] = data & 0xFF;
                    buffer[addr++] = data >> 8;
                }
            }
        }
        else if (tokens[0] == "JSR")
        {
            if(tokens[1][0] == '#') // ABSOLUTE
            {
                char* addr_c = new char[tokens[1].size()];
                for(int i = 1; tokens[1][i] != 'h' && i < tokens[1].size(); i++)
                {
                    addr_c[i - 1] = tokens[1][i];
                }
                buffer[addr++] = ins.JSR;
                buffer[addr++] = ((uint8_t)strtol(addr_c, NULL, 16));
                buffer[addr++] = ((uint8_t)strtol(addr_c, NULL, 16) >> 8);
            }
            else if (tokens[1][0] == '%')
            {
                char* label_c = new char[tokens[1].size()];
                for(int i = 1; tokens[1][i] != '%'; i++)
                {
                    if(i > tokens[1].size())
                    {
                        printf("Expected '%%' after LDA\n");
                        exit(1);
                    }
                    label_c[i - 1] = tokens[1][i];
                }
                buffer[addr++] = ins.JSR;
                buffer[addr++] = labels[label_c] & 0xFF;
                buffer[addr++] = (labels[label_c] >> 8) & 0xFF;
            }
        }
        else if(tokens[0] == "RET")
        {
            buffer[addr++] = ins.RET;
        }
        else if(tokens[0] == "PUSHA")
        {
            buffer[addr++] = ins.PHA;
        }
        else if(tokens[0] == "POPA")
        {
            buffer[addr++] = ins.PLA;
        }
        else if(tokens[0] == "TAX")
        {
            buffer[addr++] = ins.TAX;
        }
        else if(tokens[0] == "TAY")
        {
            buffer[addr++] = ins.TAY;
        }
        else if(tokens[0] == "TXA")
        {
            buffer[addr++] = ins.TXA;
        }
        else if(tokens[0] == "TXS")
        {
            buffer[addr++] = ins.TXS;
        }
        else if(tokens[0] == "TYA")
        {
            buffer[addr++] = ins.TYA;
        }
        else if(tokens[0] == "TSX")
        {
            buffer[addr++] = ins.TSX;
        }
        else if(tokens[0] == "INX")
        {
            buffer[addr++] = ins.INX;
        }
        else if(tokens[0] == "INY")
        {
            buffer[addr++] = ins.INY;
        }
        else if (tokens[0] == "DEX")
        {
            buffer[addr++] = ins.DEX;
        }
        else if(tokens[0] == "DEY")
        {
            buffer[addr++] = ins.DEY;
        }
        else if(tokens[0][0] == '`')
        {
            char* label_c = new char[tokens[0].size()];
            int c = 0;
            bool EXCL_tokenfound = false;
            for(int i = 1; tokens[0][i] != '`'; i++)
            {
                if(i > tokens[0].size())
                {
                    printf("Expected '`' after label\n");
                    exit(1);
                }
                if(tokens[0][i] != '!')
                {
                    label_c[c++] = tokens[0][i];
                }
                else
                {
                    EXCL_tokenfound = true;
                }
                
            }
            if(EXCL_tokenfound)
            {
                printf("DD_JMP: %d\n", DD_JMP);
                labels[label_c] = addr + DD_JMP;
                printf("LABEL: %s --> 0x%x\n", label_c, addr + DD_JMP);
            }
            else
            {
                labels[label_c] = addr;
                printf("LABEL: %s --> 0x%x\n", label_c, addr);
            }
        }
        else if(tokens[0] == "//")
        {
            /*
            for(int i = 1; i < tokens.size() && tokens.back() != tokens[i - 1] ; i++)
            {
                printf("%s ", tokens[i].c_str());
            }
            printf("\n");
            */
        }
        else if(tokens.size() <= 0 || tokens[0].size() <= 0 || tokens[0] == "\n" || tokens[0] == " " || tokens[0] == "\0" || tokens[0] == "\r" )
        {
        }
        else if(tokens[0][0] == '@') // Preprocessor Directives
        {
            char* p_dirv = new char[tokens[0].size()];
            int i;
            memset(p_dirv, 0, tokens[0].size());
            for(i = 1; i < tokens[0].size(); i++)
            {
                p_dirv[i - 1] = tokens[0][i];
            }
            p_dirv[i] = '\0';

            if(strcmp(p_dirv, "ERROR") == 0)
            {
                printf(RED "%s:%d: error: @ERROR ", filename, linenum);
                for(i = 1; i < tokens.size(); i++)
                {
                    if(tokens[i][0] == '*')
                    {
                        char* str = new char[tokens[i].size()];
                        for(int x = 1; tokens[i][x] != '*' && tokens[i].size() > x; x++)
                        {
                            str[x - 1] = tokens[i][x];
                        }

                        printf("%s ", definitions[str].c_str());
                    }
                    else
                        printf("%s ", tokens[i].c_str());
                }
                printf("\n" RESET);
                delete p_dirv;
                exit(1);
            }
            else if(strcmp(p_dirv, "WARN") == 0)
            {
                printf(YEL "%s:%d: warning: @WARN ", filename, linenum);
                for(i = 1; i < tokens.size(); i++)
                {
                    if(tokens[i][0] == '*')
                    {
                        char* str = new char[tokens[i].size()];
                        for(int x = 1; tokens[i][x] != '*' && tokens[i].size() > x; x++)
                        {
                            str[x - 1] = tokens[i][x];
                        }

                        printf("%s ", definitions[str].c_str());
                    }
                    else
                        printf("%s ", tokens[i].c_str());
                }
                printf("\n" RESET);
            }
            else if(strcmp(p_dirv, "INFO") == 0)
            {
                printf(CYN "%s:%d: info: @INFO ", filename, linenum);
                for(i = 1; i < tokens.size(); i++)
                {
                    if(tokens[i][0] == '*')
                    {
                        char* str = new char[tokens[i].size()];
                        for(int x = 1; tokens[i][x] != '*' && tokens[i].size() > x; x++)
                        {
                            str[x - 1] = tokens[i][x];
                        }

                        printf("%s ", definitions[str].c_str());
                    }
                    else
                        printf("%s ", tokens[i].c_str());
                }
                printf("\n" RESET);
            }
            else if(strcmp(p_dirv, "DEFINE") == 0)
            {
                char* c = new char[tokens[1].size()];
                for(i = 0; i < tokens[1].size(); i++)
                {
                    c[i] = tokens[1][i];
                }
                c[i] = '\0';

                char* val = new char[tokens[2].size()];
                for(i = 0; i < tokens[2].size(); i++)
                {
                    val[i] = tokens[2][i];
                }
                val[i] = '\0';
                definitions[c] = val;
                delete val;
                delete c;
            }
            else if(strcmp(p_dirv, "IFDEF") == 0)
            {
                char* c = new char[tokens[1].size()];
                for(i = 0; i < tokens[1].size(); i++)
                {
                    c[i] = tokens[1][i];
                }
                c[i] = '\0';
                if(definitions[c].size() > 0)
                {
                    isDefined = 2;
                }
                else
                {
                    isDefined = 1;
                }
            }
            else if(strcmp(p_dirv, "IFNDEF") == 0)
            {
                char* c = new char[tokens[1].size()];
                for(i = 0; i < tokens[1].size(); i++)
                {
                    c[i] = tokens[1][i];
                }
                c[i] = '\0';
                if(definitions[c].size() > 0)
                {
                    isNDefined = 1;
                }
                else
                {
                    isNDefined = 2;
                }
            }
            else if(strcmp(p_dirv, "MCMP") == 0)
            {
                char* c = new char[tokens[1].size()];
                for(i = 0; i < tokens[1].size(); i++)
                {
                    c[i] = tokens[1][i];
                }
                c[i] = '\0';

                char* val = new char[tokens[2].size()];
                for(i = 0; i < tokens[2].size(); i++)
                {
                    val[i] = tokens[2][i];
                }
                val[i] = '\0';

                char* val1 = new char[tokens[2].size()];
                if(val[0] == '*')
                {
                    for(i = 1; val[i] != '*' && i < strlen(val); i++)
                    {
                        val1[i - 1] = val[i];
                    }
                    val1[i] = '\0';

                    if(definitions[c] == definitions[val1])
                    {
                        isMCMP = 2;
                    }
                    else
                    {
                        isMCMP = 1;
                    }
                }
                else
                {
                    if(definitions[c] == val)
                    {
                        isMCMP = 2;
                    }
                    else
                    {
                        isMCMP = 1;
                    }
                }
                delete val1;
                delete val;
                delete c;
            }

            delete p_dirv;
        }
        else
        {
            printf(RED "%s:%d: error: Unknown keyword (%s)\n" RESET, filename, linenum, tokens[0].c_str());
            exit(1);
        }
    }
    out.write((char*)buffer, sizeof(buffer));
    out.close();
    in.close();
}

