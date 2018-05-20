cmake_minimum_required(VERSION 3.9)
project(sysopy)

set(CMAKE_CXX_STANDARD 11)

add_executable(sysopy
        zestaw1/zad1/library.c
        zestaw1/zad1/library.h
        zestaw1/zad2/library.c
        zestaw1/zad2/library.h
        zestaw1/zad2/main.c
        zestaw1/zad3a/library.c
        zestaw1/zad3a/library.h
        zestaw1/zad3a/main.c
        zestaw1/zad3a/main_dynamic.c
        zestaw1/zad3b/library.c
        zestaw1/zad3b/library.h
        zestaw1/zad3b/main.c
        zestaw1/zad3b/main_dynamic.c
        zestaw2/zad1/main.c
        zestaw2/zad2/main.c
        zestaw3/zad1/main.c
        zestaw3/zad2/main.c
        zestaw3/zad3/big_cpu.c
        zestaw3/zad3/big_data.c
        zestaw3/zad3/main.c zestaw4/zad3/main.c
        zestaw5/zad1/main.c
        zestaw5/zad2/master.c
        zestaw5/zad2/slave.c
        zestaw7/zad1/barber.c
        zestaw7/zad1/handler_of_clients.c
        zestaw7/zad1/shared_utils.h
        zestaw7/zad2/barber.c
        zestaw7/zad2/handler_of_clients.c
        zestaw7/zad2/shared_utils.h

        )
