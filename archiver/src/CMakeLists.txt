add_executable(
        archiver
        archiver.cpp
        HaffmanTree.cpp
        Stream.cpp
        Compressor.cpp
        Decompressor.cpp)

add_catch(tester_archiver Tester.cpp HaffmanTree.cpp Stream.cpp Compressor.cpp Decompressor.cpp)
