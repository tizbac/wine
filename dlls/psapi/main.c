//K32GetModuleFileNameExW
#include <stdarg.h>
#include "winerror.h"
#include "windef.h"
#include "winbase.h"


DWORD WINAPI K32GetModuleFileNameExW(HANDLE process, HMODULE module,
                                     LPWSTR file_name, DWORD size);
DWORD WINAPI GetModuleFileNameExW(HANDLE process, HMODULE module,
                                     LPWSTR file_name, DWORD size)
{
    return K32GetModuleFileNameExW(process,module,file_name,size);

}
