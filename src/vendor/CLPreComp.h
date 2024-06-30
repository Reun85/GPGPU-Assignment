#pragma once
#ifndef CL_HPP_ENABLE_EXCEPTIONS
#define CL_HPP_ENABLE_EXCEPTIONS
#endif
#define CL_TARGET_OPENCL_VERSION 300
#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_HPP_MINIMUM_OPENCL_VERSION 200
#include <CL/cl_gl.h>

#include <CL/opencl.hpp>
#include <exception>
#include <optional>
#include <sstream>

#include "oclutils.hpp"
// Attaches string to an error
class CustomCLError : public std::exception {
 public:
  CustomCLError(const cl::Error& _err, const std::string& _customMessage)
      : error(_err), customMessage(_customMessage) {}
  CustomCLError(const cl::Error& _err)
      : error(_err), customMessage(std::nullopt) {}
  CustomCLError(const std::string& _customMessage)
      : error(std::nullopt), customMessage(_customMessage) {}
  CustomCLError& operator=(const CustomCLError& _err) {
    error = _err.error;
    customMessage = _err.customMessage;
  }
  virtual const char* what() const noexcept override {
    std::ostringstream oss;
    if (error.has_value()) oss << error->what() << ": ";
    if (customMessage.has_value()) oss << *customMessage;
    whatBuffer = oss.str();
    return whatBuffer.c_str();
  }
  inline cl_int err() const { return error->err(); }

 private:
  std::optional<cl::Error> error;
  std::optional<std::string> customMessage;
  mutable std::string whatBuffer;
};
