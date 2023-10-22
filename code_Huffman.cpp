// code_Huffman.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.


#include <iostream>
#include <Windows.h>
#include <fstream>
#include <vector>
#include <string>

struct Bool_vector { // булев вектор для кодов
    long long value = 0; int size = 0;
};
std::ostream& operator << (std::ostream& os, const Bool_vector& a) { // перегрузка оператора вывода для него
    for (int i = a.size - 1; i >= 0; --i) {
        if (a.value & ((long long)1 << i)) { os << '1'; }
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
    long long t = code[local_len - 1].value + code[local_len - 2].value; // сумма последних ячеек
    int j = local_len - 2, s;
    while (j > 0 && t >= code[j - 1].value) { //прямой вставкой вставляем сумму на свое место с сохранением упорядоченности >= для красивого результата)
        code[j] = code[j - 1];
        --j;
    }
    code[j].value = t;
    get_code(code, local_len - 1); // запускаем рекурсию
    t = code[j].value; // после возврата из ячейки забираем значение кода
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


int main()
{
    SetConsoleCP(1251); // устанавливаем кодировку для ввода\вывода на консоль
    SetConsoleOutputCP(1251);
    std::string file_link; 
    std::cout << "Пожалуйста введите путь до файла: ";
    std::getline(std::cin, file_link);

    std::vector<Bool_vector> encrypting_table(256); // изначально используется для подсчета встреченых символов
    std::ifstream fin(file_link);       // открываем файл, считаем встреченые символы, закрываем
    char c;
    while (fin.get(c)) {
        encrypting_table[(byte)c].value += 1;
    }
    fin.close();

    std::vector<int> probability; // массивы под количество встреченых символов и их значение (содержат только встереченные, порядок элементов совпадает) 
    std::vector<char> signs; 

    for (int i = 0; i < (int)encrypting_table.size(); ++i) {
        if (encrypting_table[i].value) {
            probability.push_back(0);
            signs.push_back(0);
            int j = (int)probability.size() - 1; // если встретили новый символ, вставляем его на свое место с сохранением убывания вероятности, и соответствия между двумя массивами  
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
    std::ofstream fout(encrypted_file_link); // открываем файл с зашифрованным текстом на запись
    fin.open(file_link);       // открываем файл с открытым текстом на чтение
    while (fin.get(c)) {
        fout << encrypting_table[(byte)c]; // запись в файл
    }
    fin.close();
    fout.close(); // закрываем файлы

    int flag;
    std::cout << "\nЕсли вы хотите расшивровать полученый файл введите 1 иначе 0: ";
    std::cin >> flag;
    if (flag) {

        long long sodt = 0, shift;
        int min_cs = code[0].size, max_cs = code[(int)code.size() - 1].size;
        for (int i = min_cs; i <= max_cs; ++i) { sodt += (long long)1 << i; } // считаем минимально необходимый размер массива под decrypting_table 
        //(коды минимальной длинны(min_cs) находятся c 0 по 2^min_cs - 1 ячйку, коды длины i = min_cs + 1 с 2^min_cs по 2^min_cs + 2^i - 1 и т.д.)
        //т.е. для получения значения по коду длины i и значения j, нужно посчитать сдвиг до области в массиве где находятся коды длины i, и взять j-ую ячейку 

        //std::cout << sodt; // тестовый вывод
        
        std::vector<int> decrypting_table(sodt, -1000);
        for (long long i = 0, j; i < (int)code.size(); ++i) { // заполняем его
            j = code[i].value;
            for (shift = min_cs; shift < code[i].size; ++shift) { j += (long long)1 << shift; } // вычисление сдвига
            decrypting_table[j] = signs[i]; 
        }

        std::string decrypted_file_link = file_link; // создаем путь к файлу с расшифрованным текстом
        decrypted_file_link.insert((int)decrypted_file_link.size() - 4, "_decrypted");
        fout.open(decrypted_file_link); // открываем файл с расшифрованным текстом на запись  
        fin.open(encrypted_file_link); // открываем файл с зашифрованным текстом на чтение
        Bool_vector buf = { 0,0 };
        shift = 0;
        while (fin.get(c)) {
            buf.value = buf.value << 1 | (long long)(c - '0'); // после каждого считаного символа обновляем значение текушего кода
            buf.size++;
            if (buf.size > min_cs) { shift += (long long)1 << (buf.size-1); } // и вычисляем сдвиг для текущей длинны
            if (buf.size >= min_cs && decrypting_table[shift + buf.value] != -1000) { // если код допустимой длинны и для него есть символ
                fout << (byte)decrypting_table[shift + buf.value]; // записываем его в файл
                buf.value = 0; // и сбрасываем значение текущего кода и сдвига для него
                buf.size = 0;
                shift = 0;
            }
        }
        fin.close();
        fout.close(); // закрываем файлы
    }
    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"
