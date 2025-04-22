// Быстрая сортировка во внешней памяти
// алгоритм отсюда https://www.slideserve.com/neka/external-quicksort
// Входной файл input.txt и output.txt должны находиться в директории с исполняемым файлом

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <set>
#include <cassert>
#include <cmath>

const int MEMORY_LIMIT = 1000;
const int NUM_PIVOTS = sqrt(MEMORY_LIMIT);

FILE* openFile(const char* name, const char* mode) {
    FILE* f = fopen(name, mode);
    if (!f) {
        perror("Ошибка при открытии файла");
        exit(1);
    }
    return f;
}

// Обычная быстрая сортировка
void quickSort(std::vector<int>& vec, int left, int right) {
    if (left >= right) return;
    int pivot = (vec[right] + vec[left] + vec[(right + left) / 2]) / 3;
    int i = left - 1;
    for (size_t j = left; j < right; ++j) {
        if (vec[j] <= pivot)
            std::swap(vec[++i], vec[j]);
    }
    std::swap(vec[i + 1], vec[right]);
    quickSort(vec, left, i);
    quickSort(vec, i + 2, right);
}

// Считывание чанка
int readChunk(FILE* in, std::vector<int>& buffer, size_t size) {
    buffer.clear();
    int x;
    for (size_t i = 0; i < size && fscanf(in, "%d", &x) == 1; ++i)
        buffer.push_back(x);
    return buffer.size();
}

// Cоздаем временные чанки, которые влезают в память, сортируем и записываем по файлам
int createSortedChunks(const char* input_file) {
    FILE* in = openFile(input_file, "r");
    std::vector<int> buffer;
    int chunkIndex = 0;
    while (readChunk(in, buffer, MEMORY_LIMIT) > 0) {
        quickSort(buffer, 0, buffer.size() - 1);
        char chunkName[64];
        sprintf(chunkName, "chunk_%d.txt", chunkIndex);
        FILE* out = openFile(chunkName, "w");
        for (size_t x : buffer) {
            fprintf(out, "%d ", x);
        }
        fclose(out);
        ++chunkIndex;
    }
    fclose(in);
    return chunkIndex;
}

// Выбор pivot'ов
std::vector<int> selectPivots(size_t numChunks, size_t dist) {
    std::vector<int> pivots_long;
    std::vector<int> chunkData;
    for (size_t i = 0; i < numChunks; ++i) {
        char fname[64];
        sprintf(fname, "chunk_%d.txt", i);
        FILE* f = openFile(fname, "r");
        int x, count = 0;
        while (fscanf(f, "%d", &x) == 1) {
            if (count % dist == 0) {
                pivots_long.push_back(x);
            }
            count++;
        }
        fclose(f);
    }
    quickSort(pivots_long, 0, pivots_long.size() - 1);
    std::vector<int> pivots;
    for (size_t i = 1; i <= NUM_PIVOTS; ++i) {
        size_t pos = (pivots_long.size() * i) / (NUM_PIVOTS + 1);
        if (pos < pivots_long.size())
            pivots.push_back(pivots_long[pos]);
    }
    return pivots;
}

// Разделяем входной файл по выбранным pivot'ам
std::vector<std::string> distributeToBuckets(const char* input_file, const std::vector<int>& pivots) {
    FILE* in = openFile(input_file, "r");
    std::vector<std::string> bucketFiles(pivots.size() + 1);
    std::vector<FILE*> buckets;
    for (size_t i = 0; i <= pivots.size(); ++i) {
        char fname[64];
        sprintf(fname, "bucket_%d.txt", i);
        bucketFiles[i] = fname;
        buckets.push_back(openFile(fname, "w"));
    }
    int x;
    while (fscanf(in, "%d", &x) == 1) {
        int i = 0;
        while (i < pivots.size() && x >= pivots[i]) i++;
        fprintf(buckets[i], "%d ", x);
    }
    for (FILE* f : buckets) fclose(f);
    fclose(in);
    return bucketFiles;
}

// непосредственно сортировка
void externalQuickSort(const char* input_file, const char* output_file) {

    size_t numChunks = createSortedChunks(input_file);
    size_t dist = MEMORY_LIMIT / NUM_PIVOTS;

    std::vector<int> pivots = selectPivots(numChunks, dist);

    std::vector<std::string> buckets = distributeToBuckets(input_file, pivots);

    FILE* out = openFile(output_file, "w");
    for (const std::string& bucketFile : buckets) {
        FILE* file = openFile(bucketFile.c_str(), "r");
        std::vector<int> data;
        while (readChunk(file, data, MEMORY_LIMIT) > 0) {
            if (data.size() <= MEMORY_LIMIT) {
                quickSort(data, 0, data.size() - 1);
                for (size_t x : data) fprintf(out, "%d ", x);
            } else {
                assert(false && "Bucket too big — need recursive call");
            }
        }
        fclose(file);
    }

    // Удаление всех временных файлов
    for (const std::string& bucketFile : buckets) {
        remove(bucketFile.c_str());
    }

    // Удаляем временные чанки
    for (size_t i = 0; i < numChunks; ++i) {
        char chunkName[64];
        sprintf(chunkName, "chunk_%d.txt", i);
        remove(chunkName);
    }

    fclose(out);
}

int main() {
    const char* input_file = "input.txt";
    const char* output_file = "output.txt";
    externalQuickSort(input_file, output_file);
}
