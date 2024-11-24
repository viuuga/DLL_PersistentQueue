#include "pch.h"
#include "PersistentQueue.h"

#include <fstream>
#include <vector>
#include <iostream>

#define MAX_BUFFERS_LINES 2500

const char* file_QueueData  = "persistentQueue.bin",
          * file_StrNumbers = "steNumData.bin",
          * tmpFile         = "tmp.bin",
          * shiftBites      = "shiftBites.bin";

// Функция для удаления строк до указанного индекса
void removeLinesBefore(int i) {
    std::ifstream ifstream_QueueData(file_QueueData, std::ios::binary);
    if (!ifstream_QueueData) {
        std::cerr << "error read QueueData file" << std::endl;
        return;
    }

    std::ofstream ofstream_tmp(tmpFile, std::ios::binary); // Временный файл для записи
    if (!ofstream_tmp) {
        std::cerr << "error create/open tmp file" << std::endl;
        return;
    }

    int currentIndex = 1;
    int dataLen;

    // Считываем длину сообщения и сразу пропускаем до нужной строки
    while (ifstream_QueueData.read(reinterpret_cast<char*>(&dataLen), sizeof(int))) {

        if (currentIndex >= i) {
            // Записываем длину и сообщение, если индекс >= i
            ofstream_tmp.write(reinterpret_cast<const char*>(&dataLen), sizeof(int));
            std::vector<char> buffer(dataLen);
            ifstream_QueueData.read(buffer.data(), dataLen);   // Чтение сообщения
            ofstream_tmp.write(buffer.data(), dataLen);        // Запись сообщения
        }
        else {
            ifstream_QueueData.seekg(dataLen, std::ios::cur);  // Пропускаем сообщение
        }
        currentIndex++;
    }

    ifstream_QueueData.close();
    ofstream_tmp.close();


    // Обнуляем файл хранящий номера линий.
    std::ofstream ofstream_StrNumbers(file_StrNumbers, std::ios::binary | std::ios::trunc);
    ofstream_StrNumbers.close();

    // Удаляем оригинальный файл и переименовываем временный файл.
    remove(file_QueueData);
    rename(tmpFile, file_QueueData);
}

// Функция для сохранения сообщения в файл
void saveStrTo_QueueData(const char* data, int dataLen) {
    std::ofstream ofstream_QueueData(file_QueueData, std::ios::binary | std::ios::app);

    if (!ofstream_QueueData) {
        std::cerr << "error create/open QueueData file" << std::endl;
        return;
    }

    // Записываем длину сообщения
    ofstream_QueueData.write(reinterpret_cast<const char*>(&dataLen), sizeof(int));
    // Записываем само сообщение
    ofstream_QueueData.write(data, dataLen);
}

// Функция для записи значения в файл
void writeValueTo_StrNumbers(int value) {
    std::ofstream ofstream_StrNumbers(file_StrNumbers, std::ios::binary);
    if (!ofstream_StrNumbers) {
        std::cerr << "error create/open StrNumbers file" << std::endl;
        return;
    }

    ofstream_StrNumbers.write(reinterpret_cast<const char*>(&value), sizeof(int));
}

// функция для считывания последнего зачения файла file_StrNumbers (номер первой не использованной строки).
bool readLastLine_StrNumbers(int& value) {
    std::ifstream ifstream_StrNumbers(file_StrNumbers, std::ios::binary);
    if (!ifstream_StrNumbers.is_open()) {
        return false;
    }


    if (!ifstream_StrNumbers.read(reinterpret_cast<char*>(&value), sizeof(int))) {
        return false;
    }

    return true; // Успешно прочитано
}

// Функция для чтения строки по номеру
char* readStrAtNum(int num, int* length) {
    std::ifstream ifstream_QueueData(file_QueueData, std::ios::binary);
    if (!ifstream_QueueData) {
        std::cerr << "error read QueueData file" << std::endl;
        *length = 0; // Устанавливаем длину в 0
        return nullptr;
    }

    // Проверка на правильный номер
    if (num < 1) {
        *length = 0;
        return nullptr;
    }

    int shift = 0;


    if (num == 1) {
        // Чтение длины сообщения
        ifstream_QueueData.read(reinterpret_cast<char*>(length), sizeof(int)); // Читаем длину сообщения
        char* buffer = new char[*length + 1];     // Выделяем память для сообщения (+1 для нуль-терминатора)
        ifstream_QueueData.read(buffer, *length); // Чтение сообщения
        buffer[*length] = '\0';                   // Добавляем нуль-терминатор

        // Записываем длину в файл смещения
        std::ofstream ofstream_shiftBites(shiftBites, std::ios::binary);
        if (ofstream_shiftBites) {
            shift = *length + sizeof(int);
            ofstream_shiftBites.write(reinterpret_cast<const char*>(&shift), sizeof(int)); // Записываем текущее смещение
        }

        return buffer; // Возвращаем считанное сообщение
    }
    else {
        std::ifstream ifstream_shiftBites(shiftBites, std::ios::binary);
        if (ifstream_shiftBites.read(reinterpret_cast<char*>(&shift), sizeof(int))) {
            ifstream_QueueData.seekg(shift, std::ios::cur); // Перемещаем указатель

            // Чтение длины сообщения
            ifstream_QueueData.read(reinterpret_cast<char*>(length), sizeof(int));
            if (ifstream_QueueData.eof() || *length <= 0) {
                *length = 0; // Устанавливаем длину в 0, если сообщение не найдено
                return nullptr;
            }

            char* buffer = new char[*length + 1];     // Выделяем память для сообщения (+1 для нуль-терминатора)
            ifstream_QueueData.read(buffer, *length); // Чтение сообщения
            buffer[*length] = '\0';                   // Добавляем нуль-терминатор

            // Обновляем смещение
            std::ofstream ofstream_shiftBites(shiftBites, std::ios::binary | std::ios::trunc);
            if (ofstream_shiftBites) {
                int newShift = shift + sizeof(int) + *length;
                ofstream_shiftBites.write(reinterpret_cast<const char*>(&newShift), sizeof(int));
            }

            return buffer; 
        }
    }

    *length = 0;   // Устанавливаем длину в 0, если не нашли сообщение
    return nullptr; 
}

// Функция загрузки сообщения из файла
char* loadStrFromFile(int* len) {
    std::ifstream ifs(file_QueueData, std::ios::binary);
    if (!ifs) {
        *len = 0;       // Устанавливаем длину в 0
        return nullptr;
    }

    // Получаем размер файла
    ifs.seekg(0, std::ios::end);   // Перемещаем указатель в конец
    size_t fileSize = ifs.tellg(); // Получаем размер файла
    if (fileSize == 0) {
        *len = 0;       // Устанавливаем длину в 0
        return nullptr;
    }
    ifs.close();

    int str_queue_num;
    if (!readLastLine_StrNumbers(str_queue_num)) {
        str_queue_num = 1; // Установите начальное значение, если не удалось прочитать
    }

    char* buffer;
    buffer = readStrAtNum(str_queue_num, len); // считываем строку с номером str_queue_num

    if (buffer != nullptr) {
        writeValueTo_StrNumbers(++str_queue_num); // Увеличиваем номер строки
    }
    else {
        *len = 0; // Устанавливаем длину в 0 если не удалось прочитать сообщение
    }

    if (str_queue_num > MAX_BUFFERS_LINES) {
        removeLinesBefore(str_queue_num); // Удаляем строки если превышаем лимит
    }

    return buffer; // Возвращаем считанное сообщение
}


extern "C" __declspec(dllexport) void  __cdecl push(const char* data, int dataLen) {
    saveStrTo_QueueData(data, dataLen);
}

extern "C" __declspec(dllexport) char* __cdecl pop(int* len) {
    return loadStrFromFile(len);
}

extern "C" __declspec(dllexport) void  __cdecl freeData(char* data) {
    delete[] data;
}
