#ifndef PERSISTENT_QUEUE_H
#define PERSISTENT_QUEUE_H

extern "C" {
    __declspec(dllexport) void __cdecl push(const char* data, int dataLen);
    __declspec(dllexport) char* __cdecl pop(int* len);
    __declspec(dllexport) void __cdecl freeData(char* data);
}

#endif // PERSISTENT_QUEUE_H

/*
��� ���������� push ��������� ���������� ������ � ����� �����

��� ���������� pop ��������� ������� ��������� ����� ������ ����������� ������
������ ��������� ����������� ���� ������� ����� ����������, ��������� ������ ������ � ������� ���� ������, 
����� ��������� ���������� �������� � ����� ������ ����������� ������, � ���������� ������.
*/