﻿#include <iostream>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <string>

struct Bool_vector { // булев вектор для кодов
    unsigned int value = 0; char size = 0;
};
std::ostream& operator << (std::ostream& os, const Bool_vector& a) { // перегрузка оператора вывода для него
    for (int i = a.size - 1; i >= 0; --i) {
        if (a.value & (1 << i)) { os << '1'; }
        else { os << '0'; }
    }
    return os;
};

void get_code(std::vector<Bool_vector>& code, int local_len) { // рекурсивная функция распределения кодов в соответствии с вероятностью встречи символа
    if (local_len <= 2) { // переход от свернутой вероятности к кодам
        for (int i = 0; i < local_len; ++i) {
            code[i].value = i;
            code[i].size = 1;
        }
        return;
    }
    unsigned int t = code[local_len - 1].value + code[local_len - 2].value; // сумма двух последних ячеек
    int j = local_len - 2, s;
    while (j > 0 && t >= code[j - 1].value) { // прямой вставкой вставляем сумму на свое место с сохранением упорядоченности; >= для красивого результата)
        code[j] = code[j - 1];
        --j;
    }
    code[j].value = t;
    get_code(code, local_len - 1); // запускаем рекурсию
    t = code[j].value; // после возврата из ячейки с кодом для суммы забираем значение кода
    s = code[j].size;
    while (j < local_len - 2) { // возвращаем остальные ячейки на свои места
        code[j] = code[j + 1];
        ++j;
    }
    code[j].value = t << 1; // разделяем код для суммы на коды для слогаемых
    code[j].size = s + 1;
    code[j + 1].value = t << 1 | 1;
    code[j + 1].size = s + 1;
    return;
}

bool check_fequal(std::string a, std::string b) { // проверка на совпадение файлов
    std::ifstream fin_a(a);       // открываем файл на чтение
    std::ifstream fin_b(b);       // открываем файл на чтение
    char c_a, c_b;
    while (fin_a.get(c_a)) {
        if (fin_b.get(c_b)) {
            if (c_a != c_b) {
                fin_a.close();
                fin_b.close();
                std::cout << "Файлы не одинаковые"; // соответствующие символы не совпадают
                return false;
            }
        }
        else {
            fin_a.close();
            fin_b.close();
            std::cout << "Файлы не одинаковые"; // второй содержит меньше символов
            return false;
        }
    }
    if (fin_b.get(c_b)) {
        fin_a.close();
        fin_b.close();
        std::cout << "Файлы не одинаковые"; // первый содержит меньше символов
        return false;
    }
    fin_a.close();
    fin_b.close();
    std::cout << "Файлы одинаковые"; // все совпало
    return true;
}

int main()
{
    SetConsoleCP(1251); // устанавливаем кодировку для ввода\вывода на консоль
    SetConsoleOutputCP(1251);
    int flag;
    std::cout << "Если вы хотите зашифровать файл введите 1 если расшифровать 0: ";
    std::cin >> flag;
    if (flag) {
        std::string file_link; 
        std::getline(std::cin, file_link); // костыль для отбрасывания лишнего энтера, cin его не видит, а getline видит) 
        std::cout << "Пожалуйста введите путь до файла: ";
        std::getline(std::cin, file_link);
        std::vector<Bool_vector> encrypting_table(256); // таблица под значения кодов для символов, изначально используется для подсчета встреченых символов для экономии памяти
        std::ifstream fin(file_link);       // открываем файл на чтение, считаем встреченые символы, закрываем
        char c;
        while (fin.get(c)) {
            encrypting_table[(byte)c].value += 1;
        }
        fin.close();

        std::vector<unsigned int> probability; // массивы под количество встреченых символов и их значение (содержат только встереченные, порядок элементов совпадает, отсортированы по убыванию количества встреченых раз) 
        std::vector<char> signs;

        for (int i = 0; i < (int)encrypting_table.size(); ++i) { // цикл по всем возможным символам
            if (encrypting_table[i].value) { // если символ встречался в тексте
                probability.push_back(0); // добовляем под него место в массивах
                signs.push_back(0);
                int j = (int)probability.size() - 1; // вставляем его на свое место с сохранением убывания вероятности, и соответствия между двумя массивами  
                while (j > 0 && encrypting_table[i].value > probability[j - 1]) {
                    probability[j] = probability[j - 1];
                    signs[j] = signs[j - 1];
                    --j;
                }
                probability[j] = encrypting_table[i].value;
                signs[j] = i;
            }
        }

        std::vector<Bool_vector> code((int)probability.size()); // массив под получение кодов символов, заполняется вероятностью их встречи в порядке убывания

        for (int i = 0; i < (int)probability.size(); ++i) {
            //std::cout << '\n' << probability[i] << '\t' << signs[i]; // тестовый вывод
            code[i].value = probability[i];
        }

        get_code(code, (int)code.size()); // получаем коды для символов
        //std::cout << '\n';

        for (int i = 0; i < (int)code.size(); ++i) {
            encrypting_table[(byte)signs[i]] = code[i]; // заполнение кодами encrypting_table(для удобства шифрования)
            //std::cout << '\n' << code[i] << '\t' << signs[i]; // тестовый вывод
        }
        //std::cout << '\n';

        std::string encrypted_file_link = file_link; // создаем путь к файлу с зашифрованным текстом
        encrypted_file_link.insert((int)encrypted_file_link.size() - 4, "_encrypted");
        encrypted_file_link.replace((int)encrypted_file_link.size() - 3, (int)encrypted_file_link.size(), "bin");
        std::ofstream fout(encrypted_file_link, std::ios_base::binary); // открываем файл с зашифрованным текстом на запись
        fin.open(file_link);       // открываем файл с открытым текстом на чтение
        char byte_c = 0;
        int byte_size = 0, size_code = code.size();
        // Записываем таблицу со значениями и кодми в файл
        fout.write((char*)&size_code, sizeof(int));
        for (int i = 0; i < size_code; i++) {
            fout.write((char*)&(code[i].value), sizeof(unsigned int));
            fout.write((char*)&(code[i].size), sizeof(char));
            fout.write((char*)&(signs[i]), sizeof(char));
        }
        // Записываем текст в файл
        while (fin.get(c)) {
            for (int j = encrypting_table[(byte)c].size; j > 0; j--) { // выводим коды побайтово 
                byte_c = byte_c << 1 | (encrypting_table[(byte)c].value >> (j - 1) & 1);
                byte_size++;
                if (byte_size == 8) {
                    fout.write((char*)&byte_c, 1);
                    byte_c = 0;
                    byte_size = 0;
                }
            }
        }
        if (byte_size) { // обработка последнего кода
            byte_c = byte_c << (8 - byte_size);
            fout.write((char*)&byte_c, 1);
            fout.write((char*)&byte_size, 1); // число полезных бит
        }
        else {
            byte_size = 8;
            fout.write((char*)&byte_size, 1); // число полезных бит
        }
        fin.close();
        fout.close(); // закрываем файлы
    }
    else {
        std::string encrypted_file_link;
        std::getline(std::cin, encrypted_file_link); // костыль для отбрасывания лишнего энтера, cin его не видит, а getline видит)
        std::cout << "Пожалуйста введите путь до файла: ";
        std::getline(std::cin, encrypted_file_link);
        std::ifstream fin(encrypted_file_link, std::ios_base::binary); // открываем файл с зашифрованным текстом на чтение
        int size_dcode;
        fin.read((char*)&size_dcode, sizeof(int));
        std::vector<Bool_vector> dcode(size_dcode);
        std::vector<char> dsigns(size_dcode);
        for (int i = 0; i < size_dcode; i++) {
            fin.read((char*)&dcode[i].value, sizeof(unsigned int));
            fin.read((char*)&dcode[i].size, sizeof(char));
            fin.read((char*)&dsigns[i], sizeof(char));
        }
        long long sodt = 0, shift;
        int min_cs = dcode[0].size, max_cs = dcode[size_dcode - 1].size;
        for (int i = min_cs; i <= max_cs; ++i) { sodt += 1 << i; } // считаем минимально необходимый размер массива под decrypting_table 
        //(коды минимальной длинны(min_cs) находятся c 0 по 2^min_cs - 1 ячйку, коды длины i = min_cs + 1 с 2^min_cs по 2^min_cs + 2^i - 1 и т.д.)
        //т.е. для получения значения по коду длины i и значения j, нужно посчитать сдвиг до области в массиве где находятся коды длины i, и взять j-ую ячейку 

        //std::cout << sodt; // тестовый вывод
        
        std::vector<int> decrypting_table(sodt, -1000);
        for (long long i = 0, j; i < size_dcode; ++i) { // заполняем его
            j = dcode[i].value;
            for (shift = min_cs; shift < dcode[i].size; ++shift) { j += 1 << shift; } // вычисление сдвига
            decrypting_table[j] = dsigns[i]; 
        }

        std::string decrypted_file_link = encrypted_file_link; // создаем путь к файлу с расшифрованным текстом
        decrypted_file_link.replace((int)decrypted_file_link.size() - 14, (int)decrypted_file_link.size(), "_decrypted.txt");
        std::ofstream fout(decrypted_file_link); // открываем файл с расшифрованным текстом на запись  
        Bool_vector buf = { 0,0 };
        shift = 0;
        char c_1, c_2, c_3;
        fin.read((char*)&c_1, 1); // считываем два доп байта для обработки предпоследнего байта, т.к. в последнем количество полезных бит
        fin.read((char*)&c_2, 1); 
        while (fin.read((char*)&c_3, 1)) { // все последнего байта текста
            for (int j = 8; j > 0; j--) { // по битам считанного байта
                buf.value = buf.value << 1 | (c_1 >> (j-1) & 1); // после каждого считаного бита обновляем значение текушего кода
                buf.size++;
                if (buf.size > min_cs) { shift += 1 << (buf.size - 1); } // и вычисляем сдвиг для текущей длинны
                if (buf.size >= min_cs && decrypting_table[shift + buf.value] != -1000) { // если код допустимой длинны и для него есть символ
                    fout << (byte)decrypting_table[shift + buf.value]; // записываем его в файл
                    buf.value = 0; // и сбрасываем значение текущего кода и сдвига для него
                    buf.size = 0;
                    shift = 0;
                }
            }
            c_1 = c_2;
            c_2 = c_3;
        }
        for (int j = 8; j > 8 - c_2; j--) { // в последнем байте текста обрабатыаем толко значащие биты
            buf.value = buf.value << 1 | (c_1 >> (j - 1) & 1); // после каждого считаного символа обновляем значение текушего кода
            buf.size++;
            if (buf.size > min_cs) { shift += 1 << (buf.size - 1); } // и вычисляем сдвиг для текущей длинны
            if (buf.size >= min_cs && decrypting_table[shift + buf.value] != -1000) { // если код допустимой длинны и для него есть символ
                fout << (byte)decrypting_table[shift + buf.value]; // записываем его в файл
                buf.value = 0; // и сбрасываем значение текущего кода и сдвига для него
                buf.size = 0;
                shift = 0;
            }
        }
        fin.close();
        fout.close(); // закрываем файлы

        std::cout << "\nЕсли вы хотите сравнить расшифрованный файл с исхдным введите 1 иначе 0: ";
        std::cin >> flag;
        if (flag) { 
            std::string file_link = encrypted_file_link; // создаем путь к файлу с исходным текстом
            file_link.replace((int)file_link.size() - 14, (int)file_link.size(), ".txt");
            check_fequal(file_link, decrypted_file_link); 
        }
    }
    return 0;
}