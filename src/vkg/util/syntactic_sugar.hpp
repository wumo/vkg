#pragma once

#include <iostream>
#include <vector>
#include <memory>
#include <sstream>
#include <random>
#include <csignal>
#include <chrono>
#include <ctime>
#include <functional>
#include <span>

namespace vkg {
using UniqueBytes = std::unique_ptr<unsigned char, std::function<void(unsigned char *)>>;
using UniqueConstBytes = std::unique_ptr<const unsigned char, std::function<void(const unsigned char *)>>;

template<typename T>
void print(T &&x) {
    std::cout << x << std::flush;
}

template<typename T>
void println(T &&x) {
    std::cout << x << std::endl;
}

template<typename T, typename... Args>
void print(T &&first, Args &&...args) {
    print(first);
    print(args...);
}

template<typename T, typename... Args>
void println(T &&first, Args &&...args) {
    print(first);
    print(args...);
    std::cout << std::endl;
}

template<typename T>
void append(std::vector<T> &dst, std::initializer_list<T> list) {
    dst.insert(dst.end(), list);
}

template<typename T>
void append(std::vector<T> &dst, const std::vector<T> &src) {
    dst.insert(dst.end(), src.begin(), src.end());
}

template<typename T>
void append(std::vector<T> &dst, std::span<T> list) {
    dst.insert(dst.end(), list.begin(), list.end());
}

template<typename T>
void concat(std::stringstream &ss, T &&s) {
    ss << s;
}

template<typename T, typename... Args>
void concat(std::stringstream &ss, T &&s, Args &&...args) {
    ss << s;
    concat(ss, args...);
}

template<typename... Args>
std::string toString(Args &&...args) {
    std::stringstream ss;
    concat(ss, args...);
    return ss.str();
}

template<typename... Args>
void debugLog(Args &&...desc) {
#ifndef NDEBUG
    std::stringstream ss;
    concat(ss, desc...);
    std::cout << "[Debug] " << ss.str() << std::endl;
#endif
}

std::time_t currentTime();

double measure(const std::function<void()> &block);
void printMeasure(const std::function<void()> &block, std::string msg = "");

bool endWith(std::string const &fullString, std::string const &ending);

template<typename... Args>
void error(Args &&...desc) {
    std::stringstream ss;
    concat(ss, desc...);

    std::cerr << "[Error] " << ss.str() << std::endl;
    //    << boost::stacktrace::stacktrace() << std::endl;
    throw std::runtime_error(ss.str());
}

template<typename... Args>
void errorIf(bool p, Args &&...desc) {
    if(p) {
        std::stringstream ss;
        concat(ss, desc...);
        std::cerr << "[Error] " << ss.str() << std::endl;
        //      << boost::stacktrace::stacktrace() << std::endl;
        throw std::runtime_error(ss.str());
    }
}

template<typename E>
constexpr typename std::underlying_type<E>::type value(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}
}
