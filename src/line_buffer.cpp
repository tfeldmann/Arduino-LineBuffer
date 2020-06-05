#include "line_buffer.h"

LineBuffer::LineBuffer(uint32_t size, uint32_t expected_line_length = 0)
{
    buf_size_ = size;
    out_size_ = expected_line_length;
    buf_ = (char *)calloc(sizeof(char), buf_size_);
    out_ = (char *)calloc(sizeof(char), out_size_);
}

LineBuffer::~LineBuffer()
{
    free(buf_);
    free(out_);
}

size_t LineBuffer::write(uint8_t character)
{
    char c = (char)character;
    if (c == '\n')
    {
        add_char(0);
        head_ = write_;
        if (buffer_full_)
        {
            clear();
        }
    }
    else
    {
        add_char(c);
    }
    return 1;
}

char *LineBuffer::peek()
{
    copy_to_output(tail_, next_line_length());
    return out_;
}

char *LineBuffer::pop()
{
    uint32_t len = next_line_length();
    copy_to_output(tail_, len);
    if (len)
    {
        zero(tail_, len);
        tail_ = (tail_ + len + 1) % buf_size_;
    }
    return out_;
}

uint32_t LineBuffer::waiting()
{
    if (buffer_full_)
        return 0;
    return delta(tail_, head_);
}

void LineBuffer::clear()
{
    head_ = 0;
    tail_ = 0;
    write_ = 0;
    memset(buf_, 0, buf_size_);
    memset(out_, 0, out_size_);
    buffer_full_ = false;
}

void LineBuffer::add_char(char c)
{
    buf_[write_] = c;
    write_ = (write_ + 1) % buf_size_;

    if (write_ == head_)
    {
        // the message is too large for the buffer. To prevent false readings we
        // set a flag to discard the next line.
        buffer_full_ = true;
    }
    if (write_ == tail_ && !buffer_full_)
    {
        // automatically overwrite previous strings if no free space is left
        uint32_t len = next_line_length();
        if (len)
        {
            zero(tail_, len);
            tail_ = (tail_ + len + 1) % buf_size_;
        }
    }
}

bool LineBuffer::resize_output_buffer(uint32_t count)
{
    // resize and leave some space for future lines
    uint32_t new_out_size = count * 1.5;
    char *new_out = (char *)realloc(out_, sizeof(char) * new_out_size);
    if (new_out)
    {
        out_ = new_out;
        out_size_ = new_out_size;
        return true;
    }

    // reallocation not succesful. Trying again with no margin.
    new_out_size = count;
    new_out = (char *)realloc(out_, sizeof(char) * new_out_size);
    if (new_out)
    {
        out_ = new_out;
        out_size_ = new_out_size;
        return true;
    }

    // reallocation failed completely
    return false;
}

void LineBuffer::copy_to_output(uint32_t start, uint32_t count)
{
    if (count > out_size_)
    {
        bool success = resize_output_buffer(count);
        if (!success)
            count = out_size_ - 1;
    }
    memset(out_, 0, out_size_);
    for (uint32_t i = 0; i < count; i++)
    {
        char c = buf_[(start + i) % buf_size_];
        out_[i] = c;
    }
}

void LineBuffer::zero(uint32_t start, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
        buf_[(start + i) % buf_size_] = 0;
}

uint32_t LineBuffer::delta(uint32_t from, uint32_t to)
{
    if (from <= to)
        return to - from;
    else
        return buf_size_ - from + to;
}

uint32_t LineBuffer::next_line_length()
{
    if (buffer_full_)
        return 0;

    for (uint32_t i = 0; i < buf_size_; i++)
    {
        uint32_t pos = (tail_ + i) % buf_size_;
        if (pos == head_)
            return 0;
        else if (buf_[pos] == 0)
            return i;
    }
    return 0;
}
