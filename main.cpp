#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <map>
#include <utility>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

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
    for (auto header: headers) {
        o << "#include " << header << std::endl;
    }
}

void emptyLine(std::ostream &o, int n = 1) {
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
void defineNamespace(std::ostream &o, const std::string &name, Func cb, int idtLevel = 0) {
    indent(o, idtLevel) << "namespace " << name << (name.empty() ? "" : " ") << "{" << std::endl;
    cb(o);
    indent(o, idtLevel) << "}" << std::endl;
}

template<typename Func>
void defineDataMap(std::ostream &o, const std::string &name, Func cb, int idtLevel) {
    indent(o, idtLevel) << "std::map<std::string, std::vector<unsigned char> > " << name << " = {" << std::endl;
    cb(o);
    indent(o, idtLevel) << "};" << std::endl;
}

template<typename Func>
void defineDataEntry(std::ostream &o, const std::string &name, Func cb, int idtLevel) {
    indent(o, idtLevel) << "{ \"" << name << "\", ";
    cb(o);
    indent(o, idtLevel) << "}," << std::endl;
}

void defineDataVector(std::ostream &o, const std::vector<uint8_t> &data, int idtLevel) {
    o << "{";
    for (std::vector<uint8_t>::size_type i = 0; i != data.size(); i++) {
        if (i % 16 == 0) {
            o << std::endl;
            indent(o, idtLevel + 1);
        }
        o << Hex(data[i]) << ", ";
    }
    o << std::endl;
    indent(o, idtLevel) << "}" << std::endl;
}

void defineData(std::ostream &o, std::vector<File> files, int idtLevel) {
    for (auto file: files) {
        std::ifstream stream(file.path, std::ios::in | std::ios::binary);
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

        if (data.size() > 0) {
            defineDataEntry(o, file.id, [&](std::ostream &o) { defineDataVector(o, data, idtLevel + 1); }, idtLevel);
        }

        stream.close();
    }
}

void defineDataAccessor(std::ostream &o, std::string mapName, int idtLevel) {
    indent(o, idtLevel) << "char* getResource(const char* resourceName) {" << std::endl;
    indent(o, idtLevel + 1) << "auto data = " << mapName << ".find(resourceName);" << std::endl;
    indent(o, idtLevel + 1) << "if (data ==  " << mapName << ".end()) {" << std::endl;
    indent(o, idtLevel + 1 + 1) << "return nullptr;" << std::endl;
    indent(o, idtLevel + 1) << "}" << std::endl;
    indent(o, idtLevel + 1) << "return (char*) data->second.data();" << std::endl;
    indent(o, idtLevel) << "}" << std::endl;
}

void generateDataClass(std::ostream &o, std::vector<File> files) {
    auto namespaceName = "BinaryData";
    auto mapName = "dataMap";

    include(o, {
            "<iostream>",
            "<vector>",
            "<map>",
            "<utility>"
    });
    emptyLine(o);

    defineNamespace(o, namespaceName, [&](std::ostream &o) {
        emptyLine(o);

        defineNamespace(o, "", [&](std::ostream &o) {
            emptyLine(o);

            defineDataMap(o, mapName, [&](std::ostream &o) {
                defineData(o, files, 3);
            }, 2);
        }, 1);

        emptyLine(o);

        defineDataAccessor(o, mapName, 1);

    }, 0);

}

fs::path strip_root(const fs::path& p) {
    const fs::path& parent_path = p.parent_path();
    if (parent_path.empty() || parent_path.string() == "/")
        return fs::path();
    else
        return strip_root(parent_path) / p.filename();
}


int main(int argc, char **argv) {
    std::vector<std::string> resourceDirs;

    po::options_description desc("Usage");

    desc.add_options()
            ("help,h", "Print help messages")
            ("dir,d", po::value<std::vector<std::string> >(&resourceDirs)->required(),
             "Specifies the resource directories");

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

    for (auto path: resourceDirs) {
        if (!fs::exists(path) && !fs::is_directory(path)) {
            std::cerr << "No such directory " << path << std::endl;
        }
        for (fs::recursive_directory_iterator end, dir(path); dir != end; ++dir ) {
            if (fs::is_regular_file(*dir)) {
                File file { strip_root(*dir).string(), (*dir).path().string() };
                files.emplace_back(file);
            }
        }
    }


    std::ofstream ofs;
    ofs.open("BinaryData.h");

    generateDataClass(ofs, files);

    ofs.close();

    return 0;
}