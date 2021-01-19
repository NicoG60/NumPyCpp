#ifndef NP_ERROR_H
#define NP_ERROR_H

#include <exception>
#include <string>

namespace np
{

/**
 * @brief The error class is an exception object in the np namespace
 */
class error : public std::exception
{
public:
    error(const std::string& msg) : std::exception(), _msg(msg) {}

    const char * what() const noexcept override { return _msg.c_str(); }

private:
    std::string _msg;
};

}

#endif // NP_ERROR_H
