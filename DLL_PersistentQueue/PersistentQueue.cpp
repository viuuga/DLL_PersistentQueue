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

// Function for deleting rows up to the specified index
void removeLinesBefore(int i) {
    std::ifstream ifstream_QueueData(file_QueueData, std::ios::binary);
    if (!ifstream_QueueData) {
        std::cerr << "error read QueueData file" << std::endl;
        return;
    }

    std::ofstream ofstream_tmp(tmpFile, std::ios::binary);
    if (!ofstream_tmp) {
        std::cerr << "error create/open tmp file" << std::endl;
        return;
    }

    int currentIndex = 1;
    int dataLen;

    // We read the length of the message and immediately skip to the desired line
    while (ifstream_QueueData.read(reinterpret_cast<char*>(&dataLen), sizeof(int))) {

        if (currentIndex >= i) {
            
            ofstream_tmp.write(reinterpret_cast<const char*>(&dataLen), sizeof(int));
            std::vector<char> buffer(dataLen);
            ifstream_QueueData.read(buffer.data(), dataLen); 
            ofstream_tmp.write(buffer.data(), dataLen); 
        }
        else {
            ifstream_QueueData.seekg(dataLen, std::ios::cur); 
        }
        currentIndex++;
    }

    ifstream_QueueData.close();
    ofstream_tmp.close();

    std::ofstream ofstream_StrNumbers(file_StrNumbers, std::ios::binary | std::ios::trunc);
    ofstream_StrNumbers.close();

    remove(file_QueueData);
    rename(tmpFile, file_QueueData);
}

// Function for saving a message to a file
void saveStrTo_QueueData(const char* data, int dataLen) {
    std::ofstream ofstream_QueueData(file_QueueData, std::ios::binary | std::ios::app);

    if (!ofstream_QueueData) {
        std::cerr << "error create/open QueueData file" << std::endl;
        return;
    }


    ofstream_QueueData.write(reinterpret_cast<const char*>(&dataLen), sizeof(int));
    ofstream_QueueData.write(data, dataLen);
}

// Function for writing a value to a file
void writeValueTo_StrNumbers(int value) {
    std::ofstream ofstream_StrNumbers(file_StrNumbers, std::ios::binary);
    if (!ofstream_StrNumbers) {
        std::cerr << "error create/open StrNumbers file" << std::endl;
        return;
    }

    ofstream_StrNumbers.write(reinterpret_cast<const char*>(&value), sizeof(int));
}

// function for reading the last value of the file_StrNumbers file (the number of the first unused line)
bool readLastLine_StrNumbers(int& value) {
    std::ifstream ifstream_StrNumbers(file_StrNumbers, std::ios::binary);
    if (!ifstream_StrNumbers.is_open()) {
        return false;
    }


    if (!ifstream_StrNumbers.read(reinterpret_cast<char*>(&value), sizeof(int))) {
        return false;
    }

    return true; 
}

// Function for reading a line by number
char* readStrAtNum(int num, int* length) {
    std::ifstream ifstream_QueueData(file_QueueData, std::ios::binary);
    if (!ifstream_QueueData) {
        std::cerr << "error read QueueData file" << std::endl;
        *length = 0;
        return nullptr;
    }

    if (num < 1) {
        *length = 0;
        return nullptr;
    }

    int shift = 0;


    if (num == 1) {
        ifstream_QueueData.read(reinterpret_cast<char*>(length), sizeof(int));
        char* buffer = new char[*length + 1];  
        ifstream_QueueData.read(buffer, *length); 
        buffer[*length] = '\0';                  

        std::ofstream ofstream_shiftBites(shiftBites, std::ios::binary);
        if (ofstream_shiftBites) {
            shift = *length + sizeof(int);
            ofstream_shiftBites.write(reinterpret_cast<const char*>(&shift), sizeof(int)); 
        }

        return buffer; 
    }
    else {
        std::ifstream ifstream_shiftBites(shiftBites, std::ios::binary);
        if (ifstream_shiftBites.read(reinterpret_cast<char*>(&shift), sizeof(int))) {
            ifstream_QueueData.seekg(shift, std::ios::cur); 

            ifstream_QueueData.read(reinterpret_cast<char*>(length), sizeof(int));
            if (ifstream_QueueData.eof() || *length <= 0) {
                *length = 0;
                return nullptr;
            }

            char* buffer = new char[*length + 1];  
            ifstream_QueueData.read(buffer, *length); 
            buffer[*length] = '\0';                  

            std::ofstream ofstream_shiftBites(shiftBites, std::ios::binary | std::ios::trunc);
            if (ofstream_shiftBites) {
                int newShift = shift + sizeof(int) + *length;
                ofstream_shiftBites.write(reinterpret_cast<const char*>(&newShift), sizeof(int));
            }

            return buffer; 
        }
    }

    *length = 0;
    return nullptr; 
}

// The function of downloading a message from a file
char* loadStrFromFile(int* len) {
    std::ifstream ifs(file_QueueData, std::ios::binary);
    if (!ifs) {
        *len = 0;
        return nullptr;
    }

    ifs.seekg(0, std::ios::end);
    size_t fileSize = ifs.tellg();
    if (fileSize == 0) {
        *len = 0;
        return nullptr;
    }
    ifs.close();

    int str_queue_num;
    if (!readLastLine_StrNumbers(str_queue_num)) {
        str_queue_num = 1;
    }

    char* buffer;
    buffer = readStrAtNum(str_queue_num, len);

    if (buffer != nullptr) {
        writeValueTo_StrNumbers(++str_queue_num); 
    }
    else {
        *len = 0;
    }

    if (str_queue_num > MAX_BUFFERS_LINES) {
        removeLinesBefore(str_queue_num);
    }

    return buffer;
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
