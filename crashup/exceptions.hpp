#pragma once
#include <exception>
#include <string>

class CrashupInitMinidumpDirpathException : public std::exception {
  virtual const char *what() const throw() {
    return "CrashHandler initialization exception: directory for minidump "
           "files does not exist and could not be "
           "created or it is not writable. \n";
  }
};

class CrashupInitializationException : public std::runtime_error {
public:
  CrashupInitializationException(const char *msg) : std::runtime_error(msg) {}
};

class TODOException : public std::runtime_error {
public:
  TODOException(const char *msg) : std::runtime_error(msg) {}
};

class NoOpenedFileException : public std::exception {
  virtual const char *what() const throw() {
    return "Cannot write to file, file was not opened correctly. \n";
  }
};

class UnexpectedHttpResponseException : public std::exception {
  virtual const char *what() const throw() {
    return "CrashUploader:uploadFinished() -- unexpected http response from "
           "the crash report server.\n";
  }
};