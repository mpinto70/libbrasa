#include <brasa/buffer/CircularReader.h>
#include <brasa/buffer/CircularWriter.h>
#include <brasa/chronus/Now.h>
#include <brasa/chronus/SleepStd.h>
#include <brasa/chronus/Waiter.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <numeric>
#include <vector>

// The structure that will be written to the buffer
struct ElapsedTime {
    uint64_t begin;
    uint64_t end;
    size_t count;
} __attribute__((packed));

constexpr uint64_t BUFFER_KEY = 0xf1ab'25e3'3562'abde; // some arbitrary unique key

constexpr size_t ELEMENTS = 1500;
constexpr size_t BUFFER_SIZE = brasa::buffer::Circular<ElapsedTime, ELEMENTS>::BUFFER_SIZE;
using BufferDataT = brasa::buffer::BufferData<ElapsedTime, ELEMENTS>;
// 2 laps and ten more elements
constexpr size_t NUM_WRITES = 2 * ELEMENTS + 10;

std::ostream& operator<<(std::ostream& out, const ElapsedTime& elapsed) {
    out << "Elapsed [" << elapsed.count << "] --> " << elapsed.end - elapsed.begin;
    return out;
}

// produces information to be shared with the consumer
void producer(uint8_t* buffer) {
    std::cout << "Entered producer\n";
    using CircularWriter = brasa::buffer::CircularWriter<ElapsedTime, ELEMENTS>;

    // Create a writer to put the shared information into it
    CircularWriter writer(buffer, BUFFER_KEY);

    // measurements are made in microseconds
    auto begin = brasa::chronus::micro_now();

    // preparing a processing that would take roughly one millisecond each lap
    brasa::chronus::Waiter waiter =
          brasa::chronus::make_waiter(brasa::chronus::micro_now, 1000, brasa::chronus::micro_sleep);
    for (size_t i = 0; i < NUM_WRITES; ++i) {
        waiter.reset();
        waiter.wait();
        const auto end = brasa::chronus::micro_now();
        // register the elapsed time and index in the shared memory
        writer.write({ begin, end, i });
        begin = end;
    }

    std::cout << "Leaving producer\n";
}

// consumes information shared from the producer
void consumer(uint8_t* buffer) {
    std::cout << "Entered consumer\n";
    using CircularReader = brasa::buffer::CircularReader<ElapsedTime, ELEMENTS>;

    // this will hold the timings
    std::vector<ElapsedTime> timings;
    timings.reserve(NUM_WRITES);

    CircularReader reader(buffer, BUFFER_KEY);

    size_t discarded = 0;
    ElapsedTime elapsed;

    // consumption has to be faster than production, to avoid dirty reads
    brasa::chronus::Waiter waiter =
          brasa::chronus::make_waiter(brasa::chronus::nano_now, 1000, brasa::chronus::nano_sleep);
    for (size_t i = 0; i < NUM_WRITES; ++i) {
        discarded = 0;
        waiter.reset();
        while (not reader.read(elapsed)) {
            ++discarded;
            waiter.wait();
        }
        timings.push_back(elapsed);
    }

    // summarizing
    std::cout << "Number of measurements: " << timings.size() << "\n";

    auto sum_elapsed = [](uint64_t init, const ElapsedTime& elapsed) {
        return elapsed.end - elapsed.begin + init;
    };
    double total = std::accumulate(timings.begin(), timings.end(), uint64_t(0), sum_elapsed);
    std::cout << "Average time elapsed: " << total / timings.size() << " us\n";

    size_t num_skipped = 0;
    for (size_t i = 0; i < timings.size(); ++i) {
        const auto& elapsed = timings[i];
        if (elapsed.count != i + num_skipped) {
            std::cout << i + num_skipped << " != " << elapsed << "\n";
            num_skipped = elapsed.count - i;
        }
    }
    std::cout << "Leaving consumer\n";
}

int main() {
    // create a memory to be shared between producer and consumer
    constexpr const char shm_name[] = "/CIRCULAR_BUFFER";
    const int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0666);
    if (shm_fd == -1) {
        std::cerr << "Error creating shared memory: " << strerror(errno) << "\n";
        return 1;
    }

    // resize the shared memory to the buffer size
    if (ftruncate(shm_fd, BUFFER_SIZE) != 0) {
        std::cerr << "Error truncating: " << strerror(errno) << "\n";
        shm_unlink(shm_name);
        return 1;
    }

    // forks the producer
    const int write_pid = fork();
    if (write_pid == 0) {
        std::cout << "Write child\n";
        void* ptr = mmap(nullptr, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (ptr == MAP_FAILED) {
            std::cerr << "Error write mmap: " << strerror(errno) << "\n";
            return 1;
        }
        producer(static_cast<uint8_t*>(ptr));
        return 0;
    }

    // give time to the producer to initialize the shared memory
    brasa::chronus::micro_sleep(2000);

    // forks the consumer
    const int read_pid = fork();
    if (read_pid == 0) {
        brasa::chronus::milli_sleep(1);
        std::cout << "Read child\n";
        void* ptr = mmap(nullptr, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (ptr == MAP_FAILED) {
            std::cerr << "Error read mmap: " << strerror(errno) << "\n";
            return 1;
        }
        consumer(static_cast<uint8_t*>(ptr));
        return 0;
    }

    // wait for consumer and producer to finish
    int status;
    waitpid(write_pid, &status, 0);
    waitpid(read_pid, &status, 0);

    // delete the shared memory
    shm_unlink(shm_name);
}
