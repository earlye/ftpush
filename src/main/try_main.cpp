#include "try_main.hpp"

#include "ftpscanner.hpp"

#include <iostream>
#include <string>

#include <libede/format.hpp>

#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

#define BUILD_NUMBER 1
#define BUILD_ID 1

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

