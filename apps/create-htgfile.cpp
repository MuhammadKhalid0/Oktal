#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <string_view>
#include <span>

#include "oktal/octree/CellOctree.hpp"
#include "oktal/io/VtkExport.hpp"

int main(int argc, char **argv)
{
    try {
        std::span<char*> args{ argv, static_cast<std::size_t>(argc) };

        if ( argc != 3 ) {
            std::cout << std::format( "Usage: {} <filepath> <descriptor>\n", args[0] );
            return 1;
        }

        const std::filesystem::path outPath{ args[1] };
        const std::string_view descriptor{ args[2] };

        auto tree = oktal::CellOctree::fromDescriptor( descriptor );
        oktal::io::vtk::exportOctree( tree, outPath );

        return 0;
    }
    catch ( const std::exception &e ) {
        std::clog << "Error: " << e.what() << '\n';
        return 1;
    }
}
