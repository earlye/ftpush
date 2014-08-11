#include "ftpscanner.hpp"

#include <libede/auto_FILE.hpp>
#include <libede/curl/auto_CURL.hpp>
#include <libede/curl/curl_easy.hpp>
#include <libede/curl/urlencode.hpp>

#include <iostream>
#include <numeric>
#include <stack>
#include <cstdio>
#include <string>

#include <fnmatch.h>
#include <sys/stat.h>

#include <FILE.h>
#include <curl/curl.h>
#include <stdexcept>

#include <boost/noncopyable.hpp>

#define BUILD_NUMBER 1
#define BUILD_ID 1


#include <libede/format.hpp>

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/program_options.hpp>


namespace po = boost::program_options;

int try_main( int argc, char** argv )
{
  std::string description = "ftpush";
  std::string version = libede::format( "BUILD_NUMBER %s BUILD_ID %s" ) % BUILD_NUMBER % BUILD_ID;
  std::vector< std::string > sites;

  po::options_description desc( "Available Options" );
  desc.add_options()
    ("help,h", "produce help message")
    ("version,v" , "display version")
    ("site" , po::value< std::vector< std::string > >( &sites ) , "input file(s)")
    ;

  po::positional_options_description p;
  p.add("site", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc,argv).options(desc).positional(p).run(),vm);
  po::notify(vm);

  if (vm.count("version") || vm.count("help") )
    {
      std::cout << (boost::format( "%s %s" ) % description % version) << std::endl;
    }

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return 0;
    }

  if (!sites.size())
    {
      std::cout << "Usage:" << argv[0] << " [options]\n";
      std::cout << desc << std::endl;
      return 0;
    }

  ftpscanner scanner;
  scanner.set_command_line_variables( vm );

  int errors = std::accumulate( sites.begin() , sites.end() , 0 , boost::bind( boost::ref( scanner ) , _1 , _2 ) );

  return 0;
}

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
