// Original Author: Konstantin Tretyakov
// License: MIT

#include <ext/stdio_filebuf.h> // NB: Specific to libstdc++


// Wrapping pipe in a class makes sure they are closed when we leave scope
class cpipe {
  private:
    int fd[2];

  public:
    inline int read_fd() const;
    inline int write_fd() const;
    cpipe();
    void close();
    ~cpipe();
};


class spawn {
  private:
    cpipe write_pipe;
    cpipe read_pipe;

  public:
    int child_pid = -1;
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char>> write_buf = NULL;
    std::unique_ptr<__gnu_cxx::stdio_filebuf<char>> read_buf = NULL;
    std::ostream stdin;
    std::istream stdout;

    spawn(const char* const argv[], bool with_path = false, const char* const envp[] = 0);
    void send_eof();
    int wait();
};
