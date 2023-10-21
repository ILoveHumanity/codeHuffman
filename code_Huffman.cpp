// code_Huffman.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.


#include <iostream>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <string>

struct Bool_vector {
    long long value = 0; int size = 0;
};
std::ostream& operator << (std::ostream& os, const Bool_vector& a) {
    for (int i = a.size - 1; i >= 0; --i) {
        if (a.value & ((long long)1 << i)) { os << '1'; }
        else { os << '0'; }
    }
    return os;
};

void get_code(std::vector<Bool_vector>& code, int local_len) {
    if (local_len <= 2) {
        for (int i = 0; i < local_len; ++i) {
            code[i].value = i;
            code[i].size = 1;
        }
        return;
    }
    long long t = code[local_len - 1].value + code[local_len - 2].value;
    int j = local_len - 2, s;
    while (j > 0 && t >= code[j - 1].value) { // >= для красивого результата)
        code[j] = code[j - 1];
        --j;
    }
    code[j].value = t;
    get_code(code, local_len - 1);
    t = code[j].value;
    s = code[j].size;
    while (j < local_len - 2) {
        code[j] = code[j + 1];
        ++j;
    }
    code[j].value = t << 1;
    code[j].size = s + 1;
    code[j + 1].value = t << 1 | 1;
    code[j + 1].size = s + 1;
    return;
}


int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    std::string file_link;
    std::cout << "Пожалуйста введите путь до файла: ";
    std::getline(std::cin, file_link);

    std::vector<Bool_vector> encrypting_table(256);
    std::ifstream fin(file_link);       // поток для чтения
    char c;
    while (fin.get(c)) {
        encrypting_table[c].value += 1;
    }
    fin.close();

    std::vector<int> probability;
    std::vector<char> signs;

    for (int i = 0; i < (int)encrypting_table.size(); ++i) {
        if (encrypting_table[i].value) {
            probability.push_back(0);
            signs.push_back(0);
            int j = (int)probability.size() - 1; // pryamaya vstvka
            while (j > 0 && encrypting_table[i].value > probability[j - 1]) {
                probability[j] = probability[j - 1];
                signs[j] = signs[j - 1];
                --j;
            }
            probability[j] = encrypting_table[i].value;
            signs[j] = i;
        }
    }

    std::vector<Bool_vector> code((int)probability.size());

    for (int i = 0; i < (int)probability.size(); ++i) {
        std::cout << '\n' << probability[i] << '\t' << signs[i]; //
        code[i].value = probability[i];
    }

    get_code(code, (int)code.size());
    std::cout << '\n';

    for (int i = 0; i < (int)code.size(); ++i) {
        encrypting_table[signs[i]] = code[i]; // заполнение кодами encrypting_table(для удобства шифрования)
        std::cout << '\n' << code[i] << '\t' << signs[i]; //
    }
    std::cout << std::endl; //

    std::string encrypted_file_link = file_link;
    encrypted_file_link.insert((int)encrypted_file_link.size() - 4, "_encrypted");
    std::ofstream fout(encrypted_file_link); // создаём объект класса ofstream для записи и связываем его с файлом    
    fin.open(file_link);       // поток для чтения
    while (fin.get(c)) {
        fout << encrypting_table[c]; // запись в файл
    }
    fin.close();
    fout.close(); // закрываем файл

    int flag;
    std::cout << "\nЕсли вы хотите расшивровать полученый файл введите 1 иначе 0: ";
    std::cin >> flag;
    if (flag) {

        long long sodt = 0, shift;
        int min_cs = code[0].size, max_cs = code[(int)code.size() - 1].size;
        for (int i = min_cs; i <= max_cs; ++i) { sodt += (long long)1 << i; }
        std::vector<char> decrypting_table(sodt, -1);
        for (long long i = 0, j; i < (int)code.size(); ++i) {
            j = code[i].value;
            for (shift = min_cs; shift < code[i].size; ++shift) { j += (long long)1 << shift; }
            decrypting_table[j] = signs[i];
        }

        std::string decrypted_file_link = file_link;
        decrypted_file_link.insert((int)decrypted_file_link.size() - 4, "_decrypted");
        fout.open(decrypted_file_link); // создаём объект класса ofstream для записи и связываем его с файлом    
        fin.open(encrypted_file_link);       // поток для чтения
        Bool_vector buf = { 0,0 };
        shift = 0;
        while (fin.get(c)) {
            buf.value = buf.value << 1 | (long long)(c - '0');
            buf.size++;
            if (buf.size > min_cs) { shift += (long long)1 << (buf.size-1); }
            if (buf.size >= min_cs && decrypting_table[shift + buf.value] != -1) {
                fout << decrypting_table[shift + buf.value]; // запись в файл
                buf.value = 0;
                buf.size = 0;
                shift = 0;
            }
        }
        fin.close();
        fout.close(); // закрываем файл
    }
    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"
