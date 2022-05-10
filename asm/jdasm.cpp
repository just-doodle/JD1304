#include "jdasm.h"

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

    vector<string> tokens;
    map<string, uint16_t> labels;

    string line;
    uint8_t buffer[1024 * 64];
    memset(buffer, 0, 1024 * 64);
    uint16_t addr = 0;
    int DD_JMP = 0;
    while (getline(in, line))
    {
        tokens = split(line.c_str());
        if (tokens.size() == 0)
            continue;

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
                for(int i = 1; tokens[1][i] != 'h' && i < tokens[1].size(); i++)
                {
                    addr_c[i - 1] = tokens[1][i];
                }
                buffer[addr++] = ins.CHAM;
                buffer[addr++] = AM_IMM;
                buffer[addr++] = ins.LDA;
                buffer[addr++] = (uint8_t)strtol(addr_c, NULL, 16);
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
                        printf("Expected '%' after LDA\n");
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
                    buffer[addr++] = ins.INP;
                    buffer[addr++] = data & 0xFF;
                    buffer[addr++] = data >> 8;
                }
            }
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
            for(int i = 1; i < tokens.size() && tokens.back() != tokens[i - 1] ; i++)
            {
                printf("%s ", tokens[i].c_str());
            }
            printf("\n");
        }
    }
    out.write((char*)buffer, sizeof(buffer));
    out.close();
    in.close();
}

