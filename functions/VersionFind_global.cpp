#include "VersionFind_global.h"


/**********************************************************************
 *                      Functions
 *********************************************************************/
unsigned int VF_FindUsbPattern(BYTE* d, unsigned int size)
{
    for(unsigned int i = 0; i < size; i++) //55534220646576696365
        if(d[i] == 0x55 && d[i + 1] == 0x53 && d[i + 2] == 0x42 && d[i + 3] == 0x20 && d[i + 4] == 0x64 && d[i + 5] == 0x65 && d[i + 6] == 0x76 && d[i + 7] == 0x69 && d[i + 8] == 0x63 && d[i + 9] == 0x65)
        {
            while(d[i] != 0)
                i--;
            return i + 1;
        }
    return 0;
}


unsigned int VF_FindAnd20Pattern(BYTE* d, unsigned int size)
{
    for(unsigned int i = 0; i < size; i++) //83E?20
        if(d[i] == 0x83 && (d[i + 1] >> 4) == 0x0E && d[i + 2] == 0x20)
            return i;
    return 0;
}


unsigned int VF_Find40000Pattern(BYTE* d, unsigned int size)
{
    for(unsigned int i = 0; i < size; i++) //00000400
        if(d[i] == 0x00 && d[i + 1] == 0x00 && d[i + 2] == 0x04 && d[i + 3] == 0x00)
            return i;
    return 0;
}

unsigned int VF_FindShrPattern(BYTE* d, unsigned int size)
{
    for(unsigned int i = 0; i < size; i++) //C1E?0?
        if(d[i] == 0xC1 && (d[i + 1] >> 4) == 0x0E && (d[i + 2] >> 4) == 0x00)
            return i;
    return 0;
}

bool VF_IsMinimalProtection(char* szFileName, ULONG_PTR va, long parSectionNumber)
{
    int offset = GetPE32Data(szFileName, parSectionNumber, UE_SECTIONRAWOFFSET);
    BYTE firstbytes[2] = {0};
    memcpy(firstbytes, (void*)(va + offset), 2);
    if(firstbytes[0] == 0x60 && firstbytes[1] == 0xE8)
        return false;
    return true;
}


void VF_FatalError(const char* szMessage, cbErrorMessage ErrorMessageCallback)
{
    ErrorMessageCallback((char*)szMessage, (char*)"Fatal Error!");
    StopDebug();
}


unsigned int VF_FindarmVersion(BYTE* d, unsigned int size)
{
    for(unsigned int i = 0; i < size; i++) //3C61726D56657273696F6E (<armVersion)
        if(d[i] == 0x3C && d[i + 1] == 0x61 && d[i + 2] == 0x72 && d[i + 3] == 0x6D && d[i + 4] == 0x56 && d[i + 5] == 0x65 && d[i + 6] == 0x72 && d[i + 7] == 0x73 && d[i + 8] == 0x69 && d[i + 9] == 0x6F && d[i + 10] == 0x6E)
        {
            while(d[i] != 0)
                i--;
            return i + 1;
        }
    return 0;
}


unsigned int VF_FindPushAddr(BYTE* d, unsigned int size, unsigned int addr)
{
    BYTE b[4] = {0};
    memcpy(b, &addr, 4);
    for(unsigned int i = 0; i < size; i++) //68XXXXXXXX
        if(d[i] == 0x68 && d[i + 1] == b[0] && d[i + 2] == b[1] && d[i + 3] == b[2] && d[i + 4] == b[3])
            return i;
    return 0;
}
