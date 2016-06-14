#pragma once
#include <exception>
#include <string>

class CrashupInitializationException : public std::runtime_error {
public:
  CrashupInitializationException(const char *msg) : std::runtime_error(msg) {}
};

class TODOException : public std::runtime_error {
public:
  TODOException(const char *msg) : std::runtime_error(msg) {}
};

class NoOpenedFileException : public std::runtime_error {
public:
  NoOpenedFileException()
      : std::runtime_error(
            "Cannot write to file, file was not opened correctly. \n") {}
};

class UnexpectedHttpResponseException : public std::runtime_error {
public:
  UnexpectedHttpResponseException()
      : std::runtime_error(
            "CrashUploader:uploadFinished() -- unexpected http response from "
            "the crash report server.\n") {}
};
