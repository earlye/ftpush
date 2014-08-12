#ifndef h96B575E2_8BEC_4CA0_81DF_A8F070EEEF06
#define h96B575E2_8BEC_4CA0_81DF_A8F070EEEF06

#include <libede/curl/curl_easy.hpp>

#include <boost/noncopyable.hpp>
#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include <boost/property_tree/ptree.hpp>

#include <map>
#include <string>
#include <vector>

class ftpscanner
  : public boost::noncopyable
{

public:
  typedef int result_type;
  typedef boost::program_options::variables_map variables_map;

  ftpscanner( );
  
  ~ftpscanner( );
  
  int process( std::string path , struct stat const* sb , int typeflag );

  int operator( )( int errors , std::string const& filename );

  void set_command_line_variables( variables_map const& vm );

  bool log_transfers() const;

  bool log_ftp_commands() const;

  bool log_skipped_files() const;

  bool log_config_files() const;

  bool log_regex_ignore_local() const;

  bool log_configured_directories() const;

  bool log_time_check() const;

  bool log_uploading_files() const;

  bool log_using_cached_time() const;

private:
  variables_map vm_;
  boost::property_tree::ptree config_;
  std::map< std::string , time_t > time_cache_;
  std::string local_dir_;
  std::string remote_dir_;
  boost::regex local_dir_regex_;
  std::string remote_dir_pattern_;
  libede::curl::curl_easy curl_;
  int files_;
  int transfers_;
  int skips_;

  std::vector< boost::regex > ignore_local_;
};

#endif
