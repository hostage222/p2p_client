#ifndef P2P_REQUESTS_H
#define P2P_REQUESTS_H

#include <cstdint>
#include <cstddef>
#include <memory>

#include <p2p_common.h>

namespace p2p {

class request
{
public:
    virtual void fill_request(buffer_type &buf, size_t &buf_size) = 0;
    virtual bool process_answer(const buffer_type &buf, size_t buf_size) = 0;
};

class get_version_request : public request
{
public:
    get_version_request(std::shared_ptr<version_type> version);

    void fill_request(buffer_type &buf, size_t &buf_size) override;
    bool process_answer(const buffer_type &buf, size_t buf_size) override;

private:
    std::shared_ptr<version_type> version;
    bool read_version(const std::string &s);
};

class register_request : public request
{
public:
    register_request(std::string phone, std::string password, std::string code,
                     std::shared_ptr<std::string> result);

    void fill_request(buffer_type &buf, size_t &buf_size) override;
    bool process_answer(const buffer_type &buf, size_t buf_size) override;

private:
    std::string phone;
    std::string password;
    std::string code;
    std::shared_ptr<std::string> result;
};

class unregister_request : public request
{
public:
    unregister_request(std::string phone, std::string password,
                       std::shared_ptr<std::string> result);

    void fill_request(buffer_type &buf, size_t &buf_size) override;
    bool process_answer(const buffer_type &buf, size_t buf_size) override;

private:
    std::string phone;
    std::string password;
    std::shared_ptr<std::string> result;
};

class autorize_request : public request
{
public:
    autorize_request(std::string phone, std::string password,
                     std::shared_ptr<std::string> result);

    void fill_request(buffer_type &buf, size_t &buf_size) override;
    bool process_answer(const buffer_type &buf, size_t buf_size) override;

private:
    std::string phone;
    std::string password;
    std::shared_ptr<std::string> result;
};

}//p2p

#endif // P2P_REQUESTS_H
