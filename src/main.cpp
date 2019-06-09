#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <map>
#include <utility>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

struct Hex {
    unsigned char c;

    explicit Hex(unsigned char c) : c(c) {}
};

struct File {
    std::string id;
    std::string path;
};

inline std::ostream &operator<<(std::ostream &o, const Hex &h) {
    return o << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int) h.c;
}

void include(std::ostream &o, const std::vector<std::string> &headers) {
    o << "#pragma once" << std::endl;
    o << std::endl;
    for (auto header: headers) {
        o << "#include " << header << std::endl;
    }
}

void new_line(std::ostream &o, int n = 1) {
    for (auto i = 0; i < n; i++) {
        o << std::endl;
    }
}

std::ostream &indent(std::ostream &o, int level) {
    for (auto i = 0; i < level; i++) {
        o << "\t";
    }
    return o;
}

template<typename Func>
void define_ns(std::ostream &o, const std::string &name, Func cb, int idt = 0) {
    indent(o, idt) << "namespace " << name << (name.empty() ? "" : " ") << "{" << std::endl;
    cb(o);
    indent(o, idt) << "}" << std::endl;
}

template<typename Func>
void define_map(std::ostream &o, const std::string &name, Func cb, int idt) {
    indent(o, idt) << "std::map<std::string, std::vector<char> > " << name << " = {" << std::endl;
    cb(o);
    indent(o, idt) << "};" << std::endl;
}

template<typename Func>
void define_entry(std::ostream &o, const std::string &name, Func cb, int idt) {
    indent(o, idt) << "{ \"" << name << "\", ";
    cb(o);
    indent(o, idt) << "}," << std::endl;
}

void define_chunks(std::ostream &o, const std::vector <uint8_t> &data, int idt) {
    o << "{";
    for (std::vector<uint8_t>::size_type i = 0; i != data.size(); i++) {
        if (i % 16 == 0) {
            o << std::endl;
            indent(o, idt + 1);
        }
        o << Hex(data[i]) << ", ";
    }
    o << std::endl;
    indent(o, idt) << "}" << std::endl;
}

void define_resources(std::ostream &o, std::vector <File> files, int idt) {
    for (auto file: files) {
        std::ifstream stream(file.path, std::ios::in | std::ios::binary);
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        if (data.size() > 0) {
            define_entry(o, file.id, [&](std::ostream &o) { define_chunks(o, data, idt + 1); }, idt);
        }

        stream.close();
    }
}

void define_getter(std::ostream &o, std::string map_name, int idt) {
    indent(o, idt) << "inline char* get(const char* name) {" << std::endl;
    indent(o, idt + 1) << "auto it = " << map_name << ".find(name);" << std::endl;
    indent(o, idt + 1) << "return it == " << map_name << ".end() ? nullptr : it->second.data();" << std::endl;
    indent(o, idt) << "}" << std::endl;
}

void define_sizeof(std::ostream &o, std::string map_name, int idt) {
    indent(o, idt) << "inline std::vector<char>::size_type size(const char* name) {" << std::endl;
    indent(o, idt + 1) << "auto it = " << map_name << ".find(name);" << std::endl;
    indent(o, idt + 1) << "return it == " << map_name << ".end() ? 0 : it->second.size();" << std::endl;
    indent(o, idt) << "}" << std::endl;
}

void generate(std::ostream &o, std::vector <File> files, std::string ns) {
    auto map_name = "data";

    include(o, {
            "<iostream>",
            "<vector>",
            "<map>",
            "<utility>"
    });
    new_line(o);

    define_ns(o, ns, [&](std::ostream &o) {
        new_line(o);

        define_ns(o, "", [&](std::ostream &o) {
            new_line(o);

            define_map(o, map_name, [&](std::ostream &o) {
                define_resources(o, files, 3);
            }, 2);
        }, 1);

        new_line(o);

        define_getter(o, map_name, 1);

        new_line(o);

        define_sizeof(o, map_name, 1);

    }, 0);

}

int main(int argc, char **argv) {
    std::vector<std::string> src_dirs;
    std::string output;
    std::string ns;

    po::options_description desc("Usage");

    desc.add_options()
            ("help,h", "Print help messages")
            ("dir,d", po::value<std::vector<std::string> >(&src_dirs)->required(),
             "Source directories")
            ("output,o", po::value<std::string>(&output)->required(),
             "Output file")
            ("namespace,n", po::value<std::string>(&ns)->required(),
             "Namespace used to expose data");

    po::variables_map vm;

    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return 0;
    }

    try {
        po::notify(vm);
    } catch (po::error &e) {
        std::cerr << e.what() << std::endl;
    }

    std::vector<File> files;

    for (auto src: src_dirs) {
        if (!fs::exists(src) && !fs::is_directory(src)) {
            std::cerr << "No such directory " << src << std::endl;
        }

        for (fs::recursive_directory_iterator end, dir(src); dir != end; ++dir) {
            if (fs::is_regular_file(*dir)) {
                auto path = (*dir).path().string();
                auto id = boost::regex_replace(path, boost::regex(src + "/*(.*)"), "$1");
                files.emplace_back(File{id, path});
            }
        }
    }


    std::ofstream ofs;
    ofs.open(output);

    generate(ofs, files, ns);

    ofs.close();

    return 0;
}