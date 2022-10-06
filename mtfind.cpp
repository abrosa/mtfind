#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>

int main() {
    boost::iostreams::mapped_file file;

    std::string file_name = "Ulysses.txt";

    file.open(file_name, boost::iostreams::mapped_file::mapmode::readwrite);

    size_t file_size = file.size();

    if (file.is_open()) {
        char* data = (char*) file.const_data();

        for (int i = 0; i < file_size; i++) {
            std::cout << *data;
            ++data;
        }

        file.close();
    }
    else {
        std::cout << "could not map the file " << file_name << std::endl;
    }
}
