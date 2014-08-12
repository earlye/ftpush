#include "try_main.hpp"
#include <iostream>
#include <stdexcept>

int main( int argc, char** argv )
{
  try
    {
      return try_main( argc, argv );
    }
  catch( std::exception& exc )
    {
      std::cerr << "Exception: " << exc.what() << std::endl;
      return 1;
    }
}
