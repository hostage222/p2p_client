#include "p2p_requests.h"

#include <cstdint>
#include <cstddef>
#include <memory>
#include <set>
#include <unordered_set>

#include <p2p_common.h>

using namespace std;

namespace p2p {

bool process_account_answer(const buffer_type &buf, size_t buf_size,
                            string operation,
                            const unordered_set<string> &valid_answers,
                            std::shared_ptr<string> result);

get_version_request::get_version_request(shared_ptr<version_type> version) :
    version{version}
{
}

void get_version_request::fill_request(buffer_type &buf, size_t &buf_size)
{
    write_command(buf, buf_size, GET_VERSION);
    finalize(buf, buf_size);
}

bool get_version_request::process_answer(const buffer_type &buf,
                                         size_t buf_size)
{
    buf_sequence buf_seq = get_buf_sequence(buf, buf_size);
    try
    {
        string s = read_string(buf_seq);
        if (s != GET_VERSION)
        {
            return false;
        }

        s = read_string(buf_seq);
        if (!read_version(s))
        {
            return false;
        }

        if (!is_empty(buf_seq))
        {
            return false;
        }

        return true;
    }
    catch (invalid_token_exception&)
    {
        return false;
    }
}

bool get_version_request::read_version(const string &s)
{
    try
    {
        *version = string_to_version(s);
        return true;
    }
    catch (invalid_version_format_exception&)
    {
        return false;
    }
}

register_request::register_request(string phone, string password, string code,
                                   std::shared_ptr<string> result) :
    phone{move(phone)}, password{move(password)}, code{move(code)},
    result{result}
{
}

void register_request::fill_request(buffer_type &buf, size_t &buf_size)
{
    write_command(buf, buf_size, AUTORIZE);
    append_param(buf, buf_size, phone);
    append_param(buf, buf_size, password);
    append_param(buf, buf_size, code);
    finalize(buf, buf_size);
}

bool register_request::process_answer(const buffer_type &buf, size_t buf_size)
{
    static const unordered_set<string> valid_answers =
    {"OK", "NEED_CODE", "INVALID_CODE", "ALREADY_EXISTS"};

    return process_account_answer(buf, buf_size, REGISTER, valid_answers,
                                  result);
}

unregister_request::unregister_request(string phone, string password,
                                       shared_ptr<string> result) :
    phone{move(phone)}, password{move(password)}, result{result}
{
}

void unregister_request::fill_request(buffer_type &buf, size_t &buf_size)
{
    write_command(buf, buf_size, UNREGISTER);
    append_param(buf, buf_size, phone);
    append_param(buf, buf_size, password);
    finalize(buf, buf_size);
}

bool unregister_request::process_answer(const buffer_type &buf, size_t buf_size)
{
    static const unordered_set<string> valid_answers =
    {"OK", "INVALID_ACTION", "INVALID_PHONE", "INVALID_PASSWORD"};

    return process_account_answer(buf, buf_size, UNREGISTER, valid_answers,
                                  result);
}

autorize_request::autorize_request(string phone, string password,
                                   shared_ptr<string> result) :
    phone{move(phone)}, password{move(password)}, result{result}
{
}

void autorize_request::fill_request(buffer_type &buf, size_t &buf_size)
{
    write_command(buf, buf_size, AUTORIZE);
    append_param(buf, buf_size, phone);
    append_param(buf, buf_size, password);
    finalize(buf, buf_size);
}

bool autorize_request::process_answer(const buffer_type &buf, size_t buf_size)
{
    static const unordered_set<string> valid_answers =
    {"OK", "INVALID_ACTION", "INVALID_PHONE", "INVALID_PASSWORD"};

    return process_account_answer(buf, buf_size, AUTORIZE, valid_answers,
                                  result);
}

bool process_account_answer(const buffer_type &buf, size_t buf_size,
                            string operation,
                            const unordered_set<string> &valid_answers,
                            std::shared_ptr<string> result)
{
    buf_sequence buf_seq = get_buf_sequence(buf, buf_size);
    try
    {
        string s = read_string(buf_seq);
        if (s != UNREGISTER)
        {
            return false;
        }

        s = read_string(buf_seq);
        if (valid_answers.find(s) == valid_answers.end())
        {
            return false;
        }

        if (!is_empty(buf_seq))
        {
            return false;
        }

        *result = s;
        return true;
    }
    catch (invalid_token_exception&)
    {
        return false;
    }
}

}//p2p
