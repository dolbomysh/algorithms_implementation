#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <cmath>
#include <set>

const int MEMORY_LIMIT = 1000;
const int NUM_PIVOTS = sqrt(MEMORY_LIMIT);
const int MAX_RECURSION_DEPTH = 5;

FILE* openFile(const char* name, const char* mode) {
    FILE* f = fopen(name, mode);
    if (!f) {
        perror("Ошибка при открытии файла");
        exit(1);
    }
    return f;
}

void quickSort(std::vector<int>& vec, int left, int right) {
    if (left >= right) return;
    int pivot = vec[left + (right - left)/2];
    int i = left, j = right;
    while (i <= j) {
        while (vec[i] < pivot) ++i;
        while (vec[j] > pivot) --j;
        if (i <= j) {
            std::swap(vec[i], vec[j]);
            ++i;
            --j;
        }
    }
    quickSort(vec, left, j);
    quickSort(vec, i, right);
}

int readChunk(FILE* in, std::vector<int>& buffer, size_t size) {
    buffer.clear();
    int x;
    for (size_t i = 0; i < size && fscanf(in, "%d", &x) == 1; ++i)
        buffer.push_back(x);
    return buffer.size();
}

int createSortedChunks(const char* input_file) {
    FILE* in = openFile(input_file, "r");
    std::vector<int> buffer;
    int chunkIndex = 0;

    while (readChunk(in, buffer, MEMORY_LIMIT) > 0) {
        quickSort(buffer, 0, buffer.size() - 1);
        char chunkName[64];
        sprintf(chunkName, "chunk_%d.txt", chunkIndex);
        FILE* out = openFile(chunkName, "w");
        for (size_t x : buffer) fprintf(out, "%d ", x);
        fclose(out);
        ++chunkIndex;
    }

    fclose(in);
    return chunkIndex;
}

std::vector<int> selectPivots(size_t numChunks, size_t dist) {
    std::vector<int> sample;
    for (size_t i = 0; i < numChunks; ++i) {
        char fname[64];
        sprintf(fname, "chunk_%zu.txt", i);
        FILE* f = openFile(fname, "r");
        int x, count = 0;
        while (fscanf(f, "%d", &x) == 1) {
            if (count++ % dist == 0) sample.push_back(x);
        }
        fclose(f);
    }

    if (!sample.empty())
        quickSort(sample, 0, sample.size() - 1);

    std::vector<int> pivots;
    for (int i = 1; i <= NUM_PIVOTS; ++i) {
        size_t idx = sample.size() * i / (NUM_PIVOTS + 1);
        if (idx < sample.size()) pivots.push_back(sample[idx]);
    }

    return pivots;
}

void distributeToBuckets(const char* input_file, const std::vector<int>& pivots,
                         const std::vector<std::string>& bucket_files) {
    FILE* in = openFile(input_file, "r");
    std::vector<FILE*> buckets;

    for (const auto& fname : bucket_files)
        buckets.push_back(openFile(fname.c_str(), "w"));

    int x;
    while (fscanf(in, "%d", &x) == 1) {
        size_t i = 0;
        while (i < pivots.size() && x > pivots[i]) ++i;
        fprintf(buckets[i], "%d ", x);
    }

    for (auto f : buckets) fclose(f);
    fclose(in);
}

void sortAndWriteToOutput(FILE* out, const std::string& filename, int recursion_level = 0) {
    FILE* in = openFile(filename.c_str(), "r");
    std::vector<int> data;
    int x;
    while (fscanf(in, "%d", &x) == 1)
        data.push_back(x);
    fclose(in);
    remove(filename.c_str());

    if (data.size() <= MEMORY_LIMIT || recursion_level >= MAX_RECURSION_DEPTH) {
        quickSort(data, 0, data.size() - 1);
        for (int v : data) fprintf(out, "%d ", v);
        return;
    }

    char tmp_input[64];
    sprintf(tmp_input, "rec_input_%d.txt", recursion_level);
    FILE* tmp = openFile(tmp_input, "w");
    for (int v : data) fprintf(tmp, "%d ", v);
    fclose(tmp);

    std::vector<int> pivots = selectPivots(1, data.size() / NUM_PIVOTS);
    std::vector<std::string> bucket_files;
    for (size_t i = 0; i <= pivots.size(); ++i) {
        char fname[64];
        sprintf(fname, "bucket_%d_%zu.txt", recursion_level, i);
        bucket_files.push_back(fname);
    }

    distributeToBuckets(tmp_input, pivots, bucket_files);
    remove(tmp_input);

    for (const auto& bucket : bucket_files) {
        sortAndWriteToOutput(out, bucket, recursion_level + 1);
        remove(bucket.c_str());
    }
}

void externalQuickSort(const char* input_file, const char* output_file) {
    size_t numChunks = createSortedChunks(input_file);
    size_t dist = MEMORY_LIMIT / NUM_PIVOTS;
    std::vector<int> pivots = selectPivots(numChunks, dist);

    std::vector<std::string> bucket_files;
    for (size_t i = 0; i <= pivots.size(); ++i) {
        char fname[64];
        sprintf(fname, "level0_bucket_%zu.txt", i);
        bucket_files.push_back(fname);
    }

    distributeToBuckets(input_file, pivots, bucket_files);

    FILE* out = openFile(output_file, "w");
    for (const auto& bucket : bucket_files) {
        sortAndWriteToOutput(out, bucket, 1);
        remove(bucket.c_str());
    }

    for (size_t i = 0; i < numChunks; ++i) {
        char chunkName[64];
        sprintf(chunkName, "chunk_%zu.txt", i);
        remove(chunkName);
    }

    fclose(out);
}

int main() {
    const char* input_file = "task3_input.txt";
    const char* output_file = "task3_output.txt";
    externalQuickSort(input_file, output_file);
    return 0;
}
