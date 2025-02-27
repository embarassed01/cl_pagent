#ifndef _LOGSTREAM_H_TLKIT__
#define _LOGSTREAM_H_TLKIT__

#include "NonCopyable.h"
#include "exports.h"
#include <assert.h>
#include <string.h>
#include <string>

namespace trantor {
    namespace detail {
        static constexpr size_t kSmallBuffer{ 4000 };
        static constexpr size_t kLargeBuffer{ 4000 * 1000 };

        template <int SIZE>
        class TRANTOR_EXPORT FixedBuffer : NonCopyable {
        public:
            FixedBuffer() : cur_(data_) { setCookie(cookieStart); }

            ~FixedBuffer() { setCookie(cookieEnd); }

            bool append(const char * /*restrict*/ buf, size_t len) {
                if ((size_t)(avail()) > len) {
                    memcpy(cur_, buf, len);
                    cur_ += len;
                    return true;
                }
                return false;
            }

            const char *data() const { return data_; }

            int length() const { return static_cast<int>(cur_ - data_); }

            // write to data_ directly
            char *current() { return cur_; }

            int avail() const { return static_cast<int>(end() - cur_); }

            void add(size_t len) { cur_ += len; }

            void reset() { cur_ = data_; }

            void zeroBuffer() { memset(data_, 0, sizeof(data_)); }

            // for used by GDB
            const char *debugString();

            void setCookie(void (*cookie)()) { cookie_ = cookie; }

            // for used by unit test
            std::string toString() const { return std::string(data_, length()); }

        private:
            const char *end() const { return data_ + sizeof data_; }

            // Must be outline function for cookies.
            static void cookieStart();
            static void cookieEnd();

            void (*cookie_)();

            char data_[SIZE];

            char *cur_;
        };
    }  // namespace detail

    class TRANTOR_EXPORT LogStream : NonCopyable {
        using self = LogStream;

    public:
        using Buffer = detail::FixedBuffer<detail::kSmallBuffer>;

        self &operator<<(bool v) {
            append(v ? "1" : "0", 1);
            return *this;
        }

        self &operator<<(short);
        self &operator<<(unsigned short);
        self &operator<<(int);
        self &operator<<(unsigned int);
        self &operator<<(long);
        self &operator<<(unsigned long);
        self &operator<<(const long long &);
        self &operator<<(const unsigned long long &);

        self &operator<<(const void *);

        self &operator<<(float &v) {
            *this << static_cast<double>(v);
            return *this;
        }

        self &operator<<(const double &);
        self &operator<<(const long double &v);

        self &operator<<(char v) {
            append(&v, 1);
            return *this;
        }

        template <int N>
        self &operator<<(const char (&buf)[N]) {
            assert(strnlen(buf, N) == N - 1);  // 要求输入内存块内容必须是以\0结束的字符串
            append(buf, N - 1);
            return *this;
        }

        self &operator<<(char *str) {
            if (str) { append(str, strlen(str)); }
            else { append("(null)", 6); }
            return *this;
        }

        self &operator<<(const char *str) {
            if (str) { append(str, strlen(str)); }
            else { append("(null)", 6); }
            return *this;
        }

        self &operator<<(const unsigned char *str) { return operator<<(reinterpret_cast<const char*>(str)); }

        self &operator<<(const std::string &v) {
            append(v.c_str(), v.size());
            return *this;
        }

        void append(const char *data, size_t len) {
            if (exBuffer_.empty()) {
                if (!buffer_.append(data, len)) {  // fail append
                    exBuffer_.append(buffer_.data(), buffer_.length());
                    exBuffer_.append(data, len);
                }
            } else { exBuffer_.append(data, len); }
        }

        const char *bufferData() const {
            if (!exBuffer_.empty()) { return exBuffer_.data(); }
            return buffer_.data();
        }

        size_t bufferLength() const {
            if (!exBuffer_.empty()) { return exBuffer_.length(); }
            return buffer_.length();
        }

        void resetBuffer() {
            buffer_.reset();
            exBuffer_.clear();
        }

    private:
        template <typename T> 
        void formatInteger(T);

        Buffer buffer_;
        std::string exBuffer_;
    };

    class TRANTOR_EXPORT Fmt {
    public:
        template <typename T> 
        Fmt(const char *fmt, T val);

        const char * data() const { return buf_; }

        int length() const { return length_; }

    private:
        char buf_[48];
        int length_;
    };

    inline LogStream &operator<<(LogStream &s, const Fmt &fmt) {
        s.append(fmt.data(), fmt.length());
        return s;
    }

}  // namespace trantor

#endif 