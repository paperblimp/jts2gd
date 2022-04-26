
// built-in
#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ios>
#include <ostream>
#include <string_view>
#include <cstdint>
#include <array>

// extern
#include "lib/CLI11.hpp"

// local
#include "event.hpp"
#include "lexer.hpp"
#include "js_parser.hpp"
#include "tree_releaser.hpp"
#include "tree_printer.hpp"
#include "cgen.hpp"



std::string read_file(const std::string& path)
{
    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path))
        panic("the file is invalid or does not exist");
    
    std::ifstream file (path);

    if (file.bad() || !file.is_open())
        panic("could not open the file");

    
    std::string output;
    output.reserve((size_t)std::filesystem::file_size(path));

    constexpr uint32_t buffer_size = 2048;
    std::array<uint8_t, buffer_size> buffer;


    while (!file.eof())
    {
        file.read((char*)buffer.data(), buffer_size);

        output.insert(
            output.end(),
            std::make_move_iterator(buffer.begin()),
            std::make_move_iterator(buffer.begin() + (ptrdiff_t)file.gcount()));
    }

    file.close();

    return output;
}


void compile_file(const std::string& input_path, const std::string& output_path, bool ptokens, bool pjs)
{

    EventHandler eh;

    const std::string source = read_file(input_path);
    auto lres = Lexer(source, eh, input_path)();

    if (ptokens)
    {
        for (auto& tk: lres)
            std::cout << tk.repr() << '\n';
        std::cout << std::flush;
    }

    if (eh.has_error())
    {
        eh.flush();
        panic("errors found during lexical analysis. aborting");
    }

    auto pres = JSParser(lres, eh)();


    if (eh.has_error())
    {
        eh.flush();
        panic("errors found during parsing, aborting");
    }
    
    if (pjs)
        std::cout << print_tree(pres) << std::endl;

    std::ofstream output_file {output_path};
    output_file << gen_gdscript(pres) << std::endl;
    output_file.close();

    release_program(pres);

    eh.flush();
}


int main(int argc, char** argv)
{

    std::vector<std::string> input_files;
    std::string output_file;
    bool print_tokens = false;
    bool print_JS = false;


    CLI::App program {"JTS2GD"};
    program.add_option("files", input_files, "files to be compiled");
    program.add_option("-o, --output", output_file, "place to put the output");
    program.add_flag("-t, --tokens", print_tokens, "print the sequence of tokens recognized by lexer");
    program.add_flag("-j, --javascript", print_JS, "print the structure recognized by the parser in Javascript, for debug purposes only");


    CLI11_PARSE(program, argc, argv);


    if (input_files.empty())
        panic("no input files");

    if (!output_file.empty() && input_files.size() > 1)
        panic("output is not supported with multiple files");


    if (input_files.size() == 1 && !output_file.empty())
    {
        compile_file(input_files.at(0), output_file, print_tokens, print_JS);
    }
    else
    {
        for (auto& file: input_files)
        {
            auto ext_start = file.find_last_of('.');
            std::string filename (file.begin(), file.begin() + ext_start);

            compile_file(file, filename + ".gd", print_tokens, print_JS);
        }
    }
}
