#pragma once

#include <stdexcept>
#include <string>

namespace aidog {

class AidogError : public std::runtime_error {
public:
    explicit AidogError(const std::string& message) : std::runtime_error(message) {}
};

class ConnectionError : public AidogError {
public:
    explicit ConnectionError(const std::string& message) : AidogError(message) {}
};

class TimeoutError : public AidogError {
public:
    explicit TimeoutError(const std::string& message) : AidogError(message) {}
};

class ProtocolError : public AidogError {
public:
    explicit ProtocolError(const std::string& message) : AidogError(message) {}
};

class UnsupportedError : public AidogError {
public:
    explicit UnsupportedError(const std::string& message) : AidogError(message) {}
};

} // namespace aidog
