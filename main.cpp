#include "JD1304.h"
#include <csignal>
#include <memory>
#include <string>
#include "asm/jdasm.h"

#warning CPU_CLOCK_FREQUENCY is not accurate.

System* sys = 0;



void signal_handler(int sig)
{
    printf("Got signal %d\n", sig);
    sys->dump();

    exit(0);
}

int main(int argc, char** argv)
{
    if(strstr(argv[0], "JDASM") != 0 || strstr(argv[0], "jdasm") != 0 || strcmp(argv[0], "jdasm") == 0 || strcmp(argv[0], "JDASM") == 0 || strcmp(argv[0], "./jdasm") == 0 || strcmp(argv[0], "./JDASM") == 0)
    {
        if(argc < 3)
        {
            printf("Usage: %s <filename> <output>\n", argv[0]);
            return 1;
        }

        assemble(argv[1], argv[2]);
        return 0;
    }
    else if(strcmp(argv[0], "JD1304") == 0 || strcmp(argv[0], "./JD1304") == 0)
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

        System syst;
        sys = &syst;
        syst.Reset();
        syst.load(data, size);
        char c;

        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGQUIT, signal_handler);
        signal(SIGSEGV, signal_handler);

        if(strcmp(argv[2], "run") == 0)
        {
            syst.run();
            goto end;
        }
        else if(strcmp(argv[2], "step") == 0)
        {
            while(c = getchar())
            {
                if (c == '\n')
                {
                    printf("Stepping\n");
                    syst.step();
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
    }
    else
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

        System syst;
        sys = &syst;
        syst.Reset();
        syst.load(data, size);
        char c;

        //signal(SIGINT, signal_handler);
        //signal(SIGTERM, signal_handler);
        //signal(SIGQUIT, signal_handler);
        //signal(SIGSEGV, signal_handler);

        if(strcmp(argv[2], "run") == 0)
        {
            syst.run();
            goto end;
        }
        else if(strcmp(argv[2], "step") == 0)
        {
            while(c = getchar())
            {
                if (c == '\n')
                {
                    printf("Stepping\n");
                    syst.step();
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
    }

end:
    sys->dump();
    return 0;
}