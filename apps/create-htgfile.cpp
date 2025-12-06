#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <iostream>
#include <string_view>

#include "oktal/octree/CellOctree.hpp"
#include "oktal/io/VtkExport.hpp"

int main(int argc, char **argv)
{
    try {
        if ( argc != 3 ) {
            std::cout << std::format( "Usage: {} <filepath> <descriptor>\n", argv[0] );
            return 1;
        }

        std::filesystem::path outPath{ argv[1] };
        std::string_view descriptor{ argv[2] };

        auto tree = oktal::CellOctree::fromDescriptor( descriptor );
        oktal::io::vtk::exportOctree( tree, outPath );

        return 0;
    }
    catch ( const std::exception &e ) {
        std::clog << "Error: " << e.what() << '\n';
        return 1;
    }
}
