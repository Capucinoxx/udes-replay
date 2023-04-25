#ifndef UDES_REPLAY_STORAGE_H
#define UDES_REPLAY_STORAGE_H

#include <fstream>
#include <iostream>

template<typename T>
class storage {
public:
  using size_type = std::uint64_t;

private:
  struct header {
    size_type size = 0;
    size_type data_size = sizeof(T);
  };
  std::string filename;
  header hdr;
  size_type read_pos;

public:
  explicit storage(std::string filename) :
      filename{std::move(filename)}, read_pos{0} {
    read_header();
    rewrite_header();
  }

  ~storage() { rewrite_header(); }

  template<class C>
  void write(C container) {
    std::ofstream file(filename, std::ios::out | std::ios::binary | std::ios::app);
    file.write(reinterpret_cast<const char *>(container->data()), container->size() * hdr.data_size);
    file.close();
    hdr.size += container->size();
  }

  template<class C>
  bool read(C container) {
    std::size_t n = container->cap();
    if (n + read_pos > hdr.size)
      n = hdr.size - read_pos;

    if (n == 0)
      return false;

    std::ifstream file(filename, std::ios::in | std::ios::binary);
    file.seekg(sizeof(hdr) + read_pos * hdr.data_size, std::ios::beg);
    file.read(reinterpret_cast<char *>(container->data()), n * hdr.data_size);
    file.close();

    container->resize(n);

    read_pos += n;

    return true;
  }

private:
  void read_header() {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    file.read(reinterpret_cast<char *>(&hdr), sizeof(hdr));
    file.close();

    if (hdr.data_size == 0)
      hdr.data_size = sizeof(T);
  }

  void rewrite_header() {
    std::fstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<const char *>(&hdr), sizeof(hdr));
    file.close();
  }
};


#endif // UDES_REPLAY_STORAGE_H