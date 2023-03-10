#include <mod/amlmod.h>
#include <mod/logger.h>
#include <samp.h>
#include <gtasa.h>

#include "scripting.h"

// Our variables
GAME_SCRIPT_THREAD* gst;
char ScriptBuf[0xFF];
uintptr_t *pdwParamVars[18];

// Functions
void (*ProcessOneCommand)(GAME_SCRIPT_THREAD*);

void InitializeScripting()
{
    gst = new GAME_SCRIPT_THREAD {0};
    SET_TO(ProcessOneCommand, aml->GetSym(hGTASA, "_ZN14CRunningScript17ProcessOneCommandEv"));
}

inline uint8_t ExecuteScriptBuf()
{
    gst->dwScriptIP = (uintptr_t)ScriptBuf;
    ProcessOneCommand(gst);
    return gst->condResult;
}

int ScriptCommand(const SCRIPT_COMMAND *pScriptCommand, ...)
{
    va_list ap;
    const char* p = pScriptCommand->params;
    va_start(ap, pScriptCommand);
    memcpy(&ScriptBuf, &pScriptCommand->opCode, 2);
    int buf_pos = 2;
    uint16_t var_pos = 0;

    for(int i = 0; i < 18; ++i) gst->dwLocalVar[i] = 0;

    while(*p)
    {
        switch(*p)
        {
            case 'i':
            case 'd':
            {
                int i = va_arg(ap, int);
                ScriptBuf[buf_pos] = 0x01;
                memcpy(&ScriptBuf[++buf_pos], &i, 4);
                buf_pos += 4;
                break;
            }
            case 'b': // Handle booleans here
            {
                int i = va_arg(ap, int)!=0 ? 1 : 0;
                ScriptBuf[buf_pos] = 0x01;
                memcpy(&ScriptBuf[++buf_pos], &i, 4);
                buf_pos += 4;
                break;
            }
            case 'v':
            case 'p':
            {
                uint32_t *v = va_arg(ap, uint32_t*);
                ScriptBuf[buf_pos] = 0x03;
                pdwParamVars[var_pos] = (uintptr_t*)v;
                gst->dwLocalVar[var_pos] = *v;
                memcpy(&ScriptBuf[++buf_pos], &var_pos, 2);
                buf_pos += 2;
                ++var_pos;
                break;
            }
            case 'f':
            {
                float f = (float)va_arg(ap, double);
                ScriptBuf[buf_pos] = 0x06;
                memcpy(&ScriptBuf[++buf_pos], &f, 4);
                buf_pos += 4;
                break;
            }
            case 's':
            {
                char* sz = va_arg(ap, char*);
                unsigned char aLen = __strlen(sz);
                ScriptBuf[buf_pos] = 0x0E;
                ScriptBuf[++buf_pos] = aLen;
                memcpy(&ScriptBuf[++buf_pos], sz, aLen);
                buf_pos += aLen;
                break;
            }
            case 'z':
            {
                ScriptBuf[buf_pos] = 0x00;
                ++buf_pos;
                break;
            }
            default:
            {
                return 0;
            }
        }
        ++p;
    }
    va_end(ap);

    int result = ExecuteScriptBuf();
    if (var_pos)
    {
        for (int i = 0; i < var_pos; ++i) *pdwParamVars[i] = gst->dwLocalVar[i];
    }

    return result;
}