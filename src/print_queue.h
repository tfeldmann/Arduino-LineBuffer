#pragma once
#include <Arduino.h>

class PrintQueue : public Print
{
public:
    PrintQueue(uint32_t size, uint32_t expected_line_length = 20);
    ~PrintQueue();
    size_t write(uint8_t character) override;
    char *peek();
    char *pop();
    uint32_t waiting();
    void clear();

private:
    char *buf_;
    char *out_;
    uint32_t buf_size_ = 0;
    uint32_t out_size_ = 0;

    uint32_t head_ = 0;
    uint32_t write_ = 0;
    uint32_t tail_ = 0;
    bool buffer_full_ = false;

    void add_char(char c);
    bool resize_output_buffer(uint32_t count);
    void copy_to_output(uint32_t start, uint32_t count);
    void zero(uint32_t start, uint32_t count);
    uint32_t delta(uint32_t from, uint32_t to);
    uint32_t next_line_length();
};
