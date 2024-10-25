/**
 * Created October 25, 2024
 *
 * The MIT License (MIT)
 * Copyright (c) 2024 K. Suwatchai (Mobizt)
 *
 *
 * Permission is hereby granted, free of charge, to any person returning a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef VALUE_CONVERTER_H
#define VALUE_CONVERTER_H
#include <Arduino.h>
#include <string>


enum realtime_database_data_type
{
    realtime_database_data_type_undefined = -1,
    realtime_database_data_type_null = 0,
    realtime_database_data_type_integer = 1,
    realtime_database_data_type_float = 2,
    realtime_database_data_type_double = 3,
    realtime_database_data_type_boolean = 4,
    realtime_database_data_type_string = 5,
    realtime_database_data_type_json = 6,
    realtime_database_data_type_array = 7
};


// https://github.com/djGrrr/Int64String
#ifdef base16char
#undef base16char
#endif

#define base16char(i) ("0123456789ABCDEF"[i])

class NumToString
{
public:
    NumToString() {}

    template <typename T = uint64_t>
    auto val(T value, bool sign = false) -> typename std::enable_if<(std::is_same<T, uint64_t>::value), String>::type
    {

        // start at position 64 (65th byte) and work backwards
        uint8_t i = 64;
        // 66 buffer for 64 characters (binary) + B prefix + \0
        char buffer[66] = {0};

        if (value == 0)
            buffer[i--] = '0';
        else
        {
            uint8_t base_multiplied = 4;
            uint16_t multiplier = 10000;

            // Five 64 bit devisions max
            while (value > multiplier)
            {
                uint64_t q = value / multiplier;
                // get remainder without doing another division with %
                uint16_t r = value - q * multiplier;

                for (uint8_t j = 0; j < base_multiplied; j++)
                {
                    uint16_t rq = r / 10;
                    buffer[i--] = base16char(r - rq * 10);
                    r = rq;
                }

                value = q;
            }

            uint16_t remaining = value;
            while (remaining > 0)
            {
                uint16_t q = remaining / 10;
                buffer[i--] = base16char(remaining - q * 10);
                remaining = q;
            }
        }

        if (sign)
            buffer[i--] = '-';

        // return String starting at position i + 1
        return String(&buffer[i + 1]);
    }

    template <typename T = int64_t>
    auto val(T value) -> typename std::enable_if<(std::is_same<T, int64_t>::value), String>::type
    {
        // if signed, make it positive
        uint64_t uvalue = value < 0 ? -value : value;
        return val(uvalue, value < 0);
    }

    template <typename T = int>
    auto val(T value) -> typename std::enable_if<(!std::is_same<T, uint64_t>::value && !std::is_same<T, int64_t>::value), String>::type
    {
        return String(value);
    }
};

struct boolean_t : public Printable
{
private:
    String buf;
    boolean_t &copy(bool rhs)
    {
        buf = rhs ? FPSTR("true") : FPSTR("false");
        return *this;
    }

public:
    boolean_t() {}
    explicit boolean_t(bool v) : buf(v ? FPSTR("true") : FPSTR("false")) {}
    const char *c_str() const { return buf.c_str(); }
    size_t printTo(Print &p) const override { return p.print(buf.c_str()); }
};

struct number_t : public Printable
{
private:
    String buf;
    NumToString num2Str;

public:
    number_t() {}
    template <typename T1 = int, typename T = int>
    explicit number_t(T1 v, T d) : buf(String(v, d)) {}
    template <typename T = int>
    explicit number_t(T o) : buf(num2Str.val(o)) {}
    const char *c_str() const { return buf.c_str(); }
    size_t printTo(Print &p) const override { return p.print(buf.c_str()); }
};

struct string_t : public Printable
{
private:
    String buf;

public:
    string_t() {}
    template <typename T = const char *>
    explicit string_t(T v)
    {
        aq(true);
        if (std::is_same<T, bool>::value)
            buf += v ? FPSTR("true") : FPSTR("false");
        else
            buf += v;
        aq();
    }
    explicit string_t(number_t v)
    {
        aq(true);
        buf += v.c_str();
        aq();
    }
    explicit string_t(boolean_t v)
    {
        aq(true);
        buf += v.c_str();
        aq();
    }
    template <typename T>
    auto operator+=(const T &rval) -> typename std::enable_if<std::is_same<T, number_t>::value || std::is_same<T, boolean_t>::value, string_t &>::type
    {
        sap();
        buf += rval.c_str();
        aq();
        return *this;
    }

    template <typename T>
    auto operator+=(const T &rval) -> typename std::enable_if<!std::is_same<T, number_t>::value && !std::is_same<T, boolean_t>::value, string_t &>::type
    {
        sap();
        buf += rval;
        aq();
        return *this;
    }

    const char *c_str() const { return buf.c_str(); }
    size_t printTo(Print &p) const override { return p.print(buf.c_str()); }
    void clear() { buf.remove(0, buf.length()); }

private:
    void sap()
    {
        String temp;
        if (buf.length())
            temp = buf.substring(1, buf.length() - 1);
        aq(true);
        buf += temp;
    }
    void aq(bool clear = false)
    {
        if (clear)
            buf.remove(0, buf.length());
        buf += '"';
    }
};

struct object_t : public Printable
{
    friend class JsonWriter;

private:
    String buf;

public:
    object_t() {}
    explicit object_t(const String &o) : buf(o) {}
    const char *c_str() const { return buf.c_str(); }
    template <typename T = const char *>
    explicit object_t(T o) : buf(String(o)) {}
    explicit object_t(boolean_t o) : buf(o.c_str()) {}
    explicit object_t(number_t o) : buf(o.c_str()) {}
    explicit object_t(string_t o) : buf(o.c_str()) {}
    explicit object_t(bool o) : buf(o ? FPSTR("true") : FPSTR("false")) {}
    size_t printTo(Print &p) const override { return p.print(buf.c_str()); }
    void clear() { buf.remove(0, buf.length()); }
    void initObject() { buf = FPSTR("{}"); };
    void initArray() { buf = FPSTR("[]"); };

private:
    explicit operator bool() const { return buf.length() > 0; }

    template <typename T = const char *>
    auto operator=(const T &rval) -> typename std::enable_if<!std::is_same<T, object_t>::value && !std::is_same<T, string_t>::value && !std::is_same<T, number_t>::value && !std::is_same<T, boolean_t>::value, object_t &>::type
    {
        buf = rval;
        return *this;
    }

    template <typename T = String>
    auto operator+=(const T &rval) -> typename std::enable_if<!std::is_same<T, object_t>::value && !std::is_same<T, string_t>::value && !std::is_same<T, number_t>::value && !std::is_same<T, boolean_t>::value, object_t &>::type
    {
        buf += rval;
        return *this;
    }

    template <typename T>
    auto operator+=(const T &rval) -> typename std::enable_if<std::is_same<T, object_t>::value || std::is_same<T, string_t>::value || std::is_same<T, number_t>::value || std::is_same<T, boolean_t>::value, object_t &>::type
    {
        buf += rval.c_str();
        return *this;
    }

    size_t length() const { return buf.length(); }
    String substring(unsigned int beginIndex, unsigned int endIndex) const { return buf.substring(beginIndex, endIndex); }
};

class ValueConverter
{
public:
    ValueConverter() {}
    ~ValueConverter() {}

    template <typename T>
    struct v_sring
    {
        static bool const value = std::is_same<T, const char *>::value || std::is_same<T, std::string>::value || std::is_same<T, String>::value;
    };

    template <typename T>
    struct v_number
    {
        static bool const value = std::is_same<T, uint64_t>::value || std::is_same<T, int64_t>::value || std::is_same<T, uint32_t>::value || std::is_same<T, int32_t>::value ||
                                  std::is_same<T, uint16_t>::value || std::is_same<T, int16_t>::value || std::is_same<T, uint8_t>::value || std::is_same<T, int8_t>::value ||
                                  std::is_same<T, double>::value || std::is_same<T, float>::value || std::is_same<T, int>::value;
    };

    template <typename T = object_t>
    auto getVal(String &buf, T value) -> typename std::enable_if<std::is_same<T, object_t>::value || std::is_same<T, string_t>::value || std::is_same<T, boolean_t>::value || std::is_same<T, number_t>::value, void>::type
    {
        buf = value.c_str();
    }

    template <typename T = const char *>
    auto getVal(String &buf, T value) -> typename std::enable_if<(v_number<T>::value || v_sring<T>::value || std::is_same<T, bool>::value) && !std::is_same<T, object_t>::value && !std::is_same<T, string_t>::value && !std::is_same<T, boolean_t>::value && !std::is_same<T, number_t>::value, void>::type
    {
        buf.remove(0, buf.length());
        if (std::is_same<T, bool>::value)
        {
            buf = value ? "true" : "false";
        }
        else
        {
            if (v_sring<T>::value)
                buf += '\"';

            NumToString num2Str;
            buf += num2Str.val(value);

            if (v_sring<T>::value)
                buf += '\"';
        }
    }

    template <typename T>
    auto to(const char *payload) -> typename std::enable_if<v_number<T>::value || std::is_same<T, bool>::value, T>::type
    {
        if (!useLength && strlen(payload) > 0)
        {
            if (getType(payload) == realtime_database_data_type_boolean)
                setBool(strcmp(payload, "true") == 0);
            else
            {
                setInt(payload);
                setFloat(payload);
            }
        }
        else
            setBool(strlen(payload));

        if (std::is_same<T, int>::value)
            return iVal.int32;
        else if (std::is_same<T, bool>::value)
            return iVal.int32 > 0;
        else if (std::is_same<T, int8_t>::value)
            return iVal.int8;
        else if (std::is_same<T, uint8_t>::value)
            return iVal.uint8;
        else if (std::is_same<T, int16_t>::value)
            return iVal.int16;
        else if (std::is_same<T, uint16_t>::value)
            return iVal.uint16;
        else if (std::is_same<T, int32_t>::value)
            return iVal.int32;
        else if (std::is_same<T, uint32_t>::value)
            return iVal.uint32;
        else if (std::is_same<T, int64_t>::value)
            return iVal.int64;
        else if (std::is_same<T, uint64_t>::value)
            return iVal.uint64;
        else if (std::is_same<T, float>::value)
            return fVal.f;
        else if (std::is_same<T, double>::value)
            return fVal.d;
        else
            return 0;
    }

    template <typename T>
    auto to(const char *payload) -> typename std::enable_if<v_sring<T>::value, T>::type
    {
        if (payload && payload[0] == '"' && payload[strlen(payload) - 1] == '"')
        {
            buf = payload + 1;
            buf[buf.length() - 1] = 0;
        }
        else
            buf = payload;

        return buf.c_str();
    }

    realtime_database_data_type getType(const char *payload)
    {
        if (strlen(payload) > 0)
        {
            size_t p1 = 0, p2 = strlen(payload) - 1;

            if (payload[p1] == '"')
                return realtime_database_data_type_string;
            else if (payload[p1] == '{')
                return realtime_database_data_type_json;
            else if (payload[p1] == '[')
                return realtime_database_data_type_array;
            // valid database response of none numeric except for ", { and [ character should be only true, false or null.
            else if (p2 > 0 && ((payload[p1] == 'f' && payload[p1 + 1] == 'a') || (payload[p1] == 't' && payload[p1 + 1] == 'r')))
                return realtime_database_data_type_boolean;
            else if (p2 > 0 && payload[p1] == 'n' && payload[p1 + 1] == 'u')
                return realtime_database_data_type_null;
            else
            {
                // response here should be numberic value
                // find the dot and check its length to determine the type
                if (String(payload).indexOf('.') != -1)
                    return p2 <= 7 ? realtime_database_data_type_float : realtime_database_data_type_double;
                else
                    // no dot, determine the type from its value
                    return atof(payload) > 0x7fffffff ? realtime_database_data_type_double : realtime_database_data_type_integer;
            }
        }

        return realtime_database_data_type_undefined;
    }

private:
    union IVal
    {
        uint64_t uint64;
        int64_t int64;
        uint32_t uint32;
        int32_t int32;
        int16_t int16;
        uint16_t uint16;
        int8_t int8;
        uint8_t uint8;
    };

    struct FVal
    {
        double d = 0;
        float f = 0;
        void setd(double v)
        {
            d = v;
            f = static_cast<float>(v);
        }

        void setf(float v)
        {
            f = v;
            d = static_cast<double>(v);
        }
    };

    String buf;
    bool trim = false;
    bool useLength = false;

    IVal iVal = {0};
    FVal fVal;

    void setBool(bool value)
    {
        if (value)
        {
            iVal = {1};
            fVal.setd(1);
        }
        else
        {
            iVal = {0};
            fVal.setd(0);
        }
    }

    void setInt(const char *value)
    {
        if (strlen(value) > 0)
        {
            char *pEnd;
            value[0] == '-' ? iVal.int64 = strtoll(value, &pEnd, 10) : iVal.uint64 = strtoull(value, &pEnd, 10);
        }
        else
            iVal = {0};
    }

    void setFloat(const char *value)
    {
        if (strlen(value) > 0)
        {
            char *pEnd;
            fVal.setd(strtod(value, &pEnd));
        }
        else
            fVal.setd(0);
    }
};

#endif