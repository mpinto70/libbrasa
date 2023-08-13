set(thread_srcs
    Rcu.cpp
    RcuReader.cpp
    RcuWriter.cpp
)

add_lib(thread thread_srcs)
