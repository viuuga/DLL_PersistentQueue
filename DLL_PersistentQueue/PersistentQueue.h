#ifndef PERSISTENT_QUEUE_H
#define PERSISTENT_QUEUE_H

extern "C" {
    __declspec(dllexport) void __cdecl push(const char* data, int dataLen);
    __declspec(dllexport) char* __cdecl pop(int* len);
    __declspec(dllexport) void __cdecl freeData(char* data);
}

#endif // PERSISTENT_QUEUE_H

/*
При выполнении push программа замисывает строку в конец файла

При выполнении pop программа сначала считывает номер первой несчитанной строки
дальше считывает колличество байт которые нужно пропустить, считывает длинну строки и наконец саму строку, 
затем обновляет побайтовое смещение и номер первой несчитанной строки, и возвращает строку.
*/