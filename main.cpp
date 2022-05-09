#include "JD1304.h"
#include <csignal>
#include <memory>
#include <string>

System* sys;

void signal_handler(int sig)
{
    printf("Got signal %d\n", sig);
    sys->dump();

    exit(0);
}

// opcode, op1, op2
// 16bit addr = op2 << 8 | op1

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        printf("Usage: %s <filename> <Run Type>\n", argv[0]);
        return 1;
    }

    printf("JD1304 emulator %s\n", JD1304_VERSION);
    Instruction ins;
    uint8_t* data;
    uint32_t size;
    FILE* fp = fopen(argv[1], "rb");
    if(!fp)
    {
        printf("Failed to open file %s\n", argv[1]);
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data = new uint8_t[size];
    fread(data, 1, size, fp);
    fclose(fp);

    System system;
    sys = &system;
    system.Reset();
    system.load(data, size);
    char c;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGSEGV, signal_handler);

    if(strcmp(argv[2], "run") == 0)
    {
        system.run();
        goto end;
    }
    else if(strcmp(argv[2], "step") == 0)
    {
        while(c = getchar())
        {
            if (c == '\n')
            {
                printf("Stepping\n");
                system.step();
            }
            else
            {
                goto end;
            }
        }
    }
    else if(strcmp(argv[2], "help") == 0)
    {
        printf("Usage: %s <filename> <Run Type>\nRun types:\nrun : To run the system with clock frequency of 10hz.\nstep : To step through to execution process on application in memory using enter.\nhelp : To display this message.\n", argv[0]);
    }
    else
    {
        printf("Unknown run type %s\nRun types:\nrun : To run the system with clock frequency of 10hz.\nstep : To step through to execution process on application in memory using enter.\nhelp : To display this message.\n", argv[2]);
        return 1;
    }

end:
    system.dump();
    return 0;
}