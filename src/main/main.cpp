#include <iostream>
#include <numeric>
#include <stack>
#include <cstdio>
#include <string>

#include <ftw.h>
#include <fnmatch.h>
#include <sys/stat.h>

//#include "build_environment.h"
#include <libede/auto_FILE.hpp>
//#include <ecpp/curl_easy.hpp>
//#include <ecpp/format.hpp>
//#include <ecpp/thread_local_singleton.hpp>
//#include <ecpp/urlencode.hpp>

#include <boost/bind.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/regex.hpp>

namespace po = boost::program_options;

template< typename TARGET_TYPE , typename SOURCE_TYPE > 
TARGET_TYPE lexical_cast( SOURCE_TYPE source , TARGET_TYPE default_value )
{
  TARGET_TYPE result = default_value;
  std::stringstream s;
  s << source;
  s.seekg( 0 );
  s >> result;
  return result;
}

int curl_debug(CURL * handle,  curl_infotype info,  char  * message, size_t msg_size,  void  * user_data)
{
  return msg_size;
}

size_t curl_header( void *ptr,  size_t  size,  size_t  nmemb,void  *userdata )
{
  return size;
}

class ftpscanner
  : public boost::noncopyable
{
private:
  typedef ecpp::thread_local_singleton< std::stack< ftpscanner* > , ftpscanner > scanner_stack;
public:
  typedef int result_type;

  ftpscanner( )
    : files_( 0 )
    , transfers_( 0 )
    , skips_( 0 )
  {
  }

  ~ftpscanner( )
  {
    std::cout << boost::format( "%d files found\n%d files transferred\n%d files skipped\n") % files_ % transfers_ % skips_;
  }

  static int callback( char const* path , struct stat const* sb , int typeflag )
  {
    ftpscanner* pthis = scanner_stack::instance().top();
    return pthis->process( path , sb , typeflag );
  }

  int process( std::string path , struct stat const* sb , int typeflag )
  {
    BOOST_FOREACH( boost::regex& v , ignore_local_ )
      {
	if ( regex_match( path , v ) )
	  {
	    if ( log_skipped_files() )
	      std::cout << boost::format( "SKIPPING: %s matches regex %s\n" ) % path % v;
	    return 0;
	  }
      }

    if ( typeflag & ( FTW_D | FTW_DNR | FTW_DP )  )
      return 0;

    ++files_;

    std::string remote_path = boost::regex_replace( path , local_dir_regex_ , remote_dir_pattern_ , boost::match_default | boost::format_sed );
    std::string username = config_.get<std::string>( "site.username" );
    std::string password = config_.get<std::string>( "site.password" );

    std::string url = ecpp::format( "ftp://%s:%s@%s/%s" )
      % ecpp::urlencode(username)
      % ecpp::urlencode(password)
      % config_.get<std::string>( "site.host" )
      % remote_path;

    std::string loggable_url = ecpp::format( "ftp://%s:***@%s/%s" )
      % ecpp::urlencode(username)
      % config_.get<std::string>( "site.host" )
      % remote_path;

    ecpp::auto_FILE local_file( std::fopen( path.c_str() , "r" ) );

    curl_.setopt_url( url );
    curl_.setopt_verbose( log_ftp_commands() );
    curl_.setopt_ftp_create_missing_dirs( true );
    curl_.setopt( CURLOPT_READDATA , local_file.get() );
    curl_.setopt( CURLOPT_READFUNCTION , std::fread );
    curl_.setopt( CURLOPT_FILETIME , 1 );

    std::time_t local_time = boost::filesystem::last_write_time( path );
    long remote_time = time_cache_[ path ];

    if ( !remote_time )
      {
	// Disable the data transfer and just get the time.
	curl_.setopt( CURLOPT_NOBODY , 1 );
	curl_.setopt( CURLOPT_FTP_FILEMETHOD , CURLFTPMETHOD_NOCWD );

#if LIBCURL_VERSION_NUM <= 0x071c00
#define CURL_RUDELY_PRINTS_TO_STDOUT
#endif

#ifdef CURL_RUDELY_PRINTS_TO_STDOUT
	fclose( stdout );
	freopen( "/dev/null" , "a" , stdout );
#endif

	CURLcode check = curl_.try_perform( );

#ifdef CURL_RUDELY_PRINTS_TO_STDOUT
	fclose( stdout );
	freopen( "/dev/tty" , "a" , stdout );
#endif

	if ( check == CURLE_OK )
	  curl_.getinfo( CURLINFO_FILETIME , &remote_time );

	time_cache_[ path ] = remote_time;
      }
    else
      {
	if ( log_using_cached_time() )
	  std::cout << boost::format( "%s using cached remote time:%d\n" ) % path % remote_time;
      }

    bool upload = local_time > remote_time;

    if ( log_time_check() )
      std::cout << boost::format( "%s local time:%d remote time:%d upload?:%d\n" )
	% path % local_time % remote_time % upload;

    if ( upload )
      {
	if ( log_uploading_files() )
	  {
	    std::cout << boost::format( "%s => %s [remote-time:%d local-time:%d]\n" )
	      % path % loggable_url % remote_time % local_time;
	  }

	curl_.setopt_upload( true );
	curl_.setopt_url( url );
	curl_.setopt( CURLOPT_NOBODY , 0 );
	curl_.setopt( CURLOPT_FILETIME , 0 );
	curl_.setopt( CURLOPT_FTP_FILEMETHOD , CURLFTPMETHOD_SINGLECWD ); 
	CURLcode check = curl_.try_perform( );

	if ( check != CURLE_OK )
	  {
	    curl_.setopt_upload( true );
	    curl_.setopt_url( url );
	    curl_.setopt_verbose( 1 );
	    curl_.setopt_ftp_create_missing_dirs( true );
	    curl_.setopt( CURLOPT_READDATA , local_file.get() );
	    curl_.setopt( CURLOPT_READFUNCTION , std::fread );
	    curl_.setopt( CURLOPT_DEBUGFUNCTION , curl_debug );
	    curl_.setopt( CURLOPT_DEBUGFUNCTION , curl_header );
	    curl_.setopt( CURLOPT_FTP_FILEMETHOD , CURLFTPMETHOD_MULTICWD );
	    curl_.perform( ); // LAST CHANCE!
	  }
	++transfers_;
      }
    else
      {
	++skips_;
      }

    time_cache_[path] = local_time;
    { std::string time_cache_file = config_.get< std::string>( "site.time-cache" , "" );
      if ( time_cache_file.length() )
	{
	  std::ofstream file( time_cache_file.c_str() , std::ios::app );
	  file << path << "\n" << local_time << "\n";
	}
    }
    return 0;
  }

  int operator( )( int errors , std::string const& filename )
  {
    if ( log_config_files( ) )
      std::cout << boost::format( "%s - %p:%s\n" ) % __PRETTY_FUNCTION__ % this % filename;

    { std::ifstream file( filename.c_str() );
      boost::property_tree::read_ini( file , config_ ); }

    { std::string time_cache_file = config_.get<std::string>( "site.time-cache" , "" );
      time_cache_.clear();
      if ( time_cache_file.length() )
	{
	  std::ifstream file( time_cache_file.c_str() );
	  while( file && !file.eof() )
	    {
	      std::string time;
	      std::string path;
	      std::getline( file , path );
	      std::getline( file , time );
	      time_cache_[ path ] = lexical_cast<time_t>(time,0);
	    }
	}
    }

    BOOST_FOREACH(boost::property_tree::ptree::value_type& v, config_.get_child("ignore-local"))
      {
	std::string regex_string = v.first;
	bool enabled = boost::lexical_cast<bool>(v.second.data());
	if ( !enabled )
	  continue;

	if ( log_regex_ignore_local() )
	  std::cout << boost::format( "%s - adding regex to ignore_local_:%s\n" ) % __PRETTY_FUNCTION__ % regex_string;
	boost::regex regex( regex_string );
	ignore_local_.push_back(regex);
      }


    scanner_stack::instance().push( this );
    BOOST_FOREACH(boost::property_tree::ptree::value_type& v, config_.get_child("directory-map"))
      {	
	static boost::regex trailing_slash( "/$" );
	static boost::regex leading_slash( "^/" );
	local_dir_ = boost::regex_replace( v.first , trailing_slash ,  "" , boost::match_default | boost::format_sed );
	remote_dir_ = boost::regex_replace( v.second.data() , trailing_slash ,  "" , boost::match_default | boost::format_sed );
	remote_dir_ = boost::regex_replace( remote_dir_ , leading_slash , "" , boost::match_default | boost::format_sed );

	if ( log_configured_directories() )
	  {
	    std::cout << boost::format( "%s - local_dir:%s\n" ) % __PRETTY_FUNCTION__ % local_dir_;
	    std::cout << boost::format( "%s - remote_dir:%s\n" ) % __PRETTY_FUNCTION__ % remote_dir_;
	  }

	local_dir_regex_ = boost::regex(local_dir_ + "(.*)$");
	remote_dir_pattern_ = remote_dir_ + "\\1";


	ftw( local_dir_.c_str() , callback , FTW_MOUNT | FTW_PHYS );
      }
    scanner_stack::instance().pop( );
    return 0;
  }

  void set_command_line_variables( po::variables_map const& vm ) { vm_ = vm; }

  bool log_transfers() const
  {
    return config_.get<bool>( "log.transfers" , 0 );
  }

  bool log_ftp_commands() const
  {
    return config_.get<bool>( "log.ftp-commands" , 0 );
  }

  bool log_skipped_files() const
  {
    return config_.get<bool>( "log.skipped-files" , 0 );
  }

  bool log_config_files() const
  {
    return vm_.count( "log-config-files" );
  }

  bool log_regex_ignore_local() const
  {
    return config_.get<bool>( "log.ignore-locals" , 0 );
  }

  bool log_configured_directories() const
  {
    return config_.get<bool>( "log.configured-directories" , 0 );
  }

  bool log_time_check() const
  {
    return config_.get<bool>( "log.time-check" , 0 );
  }

  bool log_uploading_files() const
  {
    return config_.get<bool>( "log.uploading-files" , 1 );
  }

  bool log_using_cached_time() const
  {
    return config_.get<bool>( "log.using-cached-time" , 0 );
  }

private:
  po::variables_map vm_;
  boost::property_tree::ptree config_;
  std::map< std::string , time_t > time_cache_;
  std::string local_dir_;
  std::string remote_dir_;
  boost::regex local_dir_regex_;
  std::string remote_dir_pattern_;
  ecpp::curl_easy curl_;
  int files_;
  int transfers_;
  int skips_;

  std::vector< boost::regex > ignore_local_;
};

int try_main( int argc, char** argv )
{
  std::string description = "ftpush";
  std::string version = ecpp::format( "BUILD_NUMBER %s BUILD_ID %s" ) % BUILD_NUMBER % BUILD_ID;
  std::vector< std::string > sites;

  po::options_description desc( "Usage" );
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
