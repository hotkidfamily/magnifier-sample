#pragma once
#include "desktop_capture/desktop_cpature_include.h"

using namespace webrtc;

const int kTestSharedMemoryId = 123;

class FakeSharedMemory : public SharedMemory {
public:
    FakeSharedMemory(char* buffer, size_t size)
        : SharedMemory(buffer, size, 0, kTestSharedMemoryId), buffer_(buffer) {}
    ~FakeSharedMemory() override { delete[] buffer_; }

private:
    char* buffer_;
    RTC_DISALLOW_COPY_AND_ASSIGN(FakeSharedMemory);
};

class FakeSharedMemoryFactory : public SharedMemoryFactory {
public:
    FakeSharedMemoryFactory() {}
    ~FakeSharedMemoryFactory() override {}

    std::unique_ptr<SharedMemory> CreateSharedMemory(size_t size) override {
        return std::unique_ptr<SharedMemory>(
            new FakeSharedMemory(new char[size], size));
    }

private:
    RTC_DISALLOW_COPY_AND_ASSIGN(FakeSharedMemoryFactory);
};
