#ifndef h8348D2AC_87AA_4666_9D3F_3F91C682480B
#define h8348D2AC_87AA_4666_9D3F_3F91C682480B

#include "auto_CURL.hpp"

#include <curl/curl.h>
#include <boost/noncopyable.hpp>

namespace libede
{
  namespace curl
  {
    class curl_easy
      : boost::noncopyable
    {
    public:
      curl_easy() { }

      CURLcode try_perform()
      {
        CURLcode result = curl_easy_perform( curl_.get() );
        return result;
      }

      void perform()
      {
        CURLcode code = try_perform();
	if (CURLE_OK==code)
	  return;
	throw std::runtime_error(curl_easy_strerror(code));
      }

      
      template< typename PARAMETER >
      CURLcode try_getinfo(CURLINFO info, PARAMETER parameter )
      {
	CURLcode result = curl_easy_getinfo( curl_.get() , info, parameter );
	return result;
      }

      template< typename PARAMETER >
      void getinfo(CURLINFO info, PARAMETER parameter )
      {
	CURLcode code = try_getinfo(info,parameter);
	if (CURLE_OK==code)
	  return;
	throw std::runtime_error(curl_easy_strerror(code));
      }

      void setopt_url( std::string const& url )
      {
        url_ = url;
        setopt( CURLOPT_URL , url_.c_str() );
      }

      void setopt_verbose( bool value )
      {
        setopt( CURLOPT_VERBOSE , value );
      }

      void setopt_ftp_create_missing_dirs( curl_ftpcreatedir value )
      {
        setopt( CURLOPT_FTP_CREATE_MISSING_DIRS , value );
      }

      void setopt_upload( bool value )
      {
	setopt( CURLOPT_UPLOAD , value );
      }

      template< typename PARAMETER >
      void setopt( CURLoption option, PARAMETER parameter )
      {
        CURLcode code = curl_easy_setopt( curl_.get() , option , parameter );
        if ( CURLE_OK == code )
          return;
        throw std::runtime_error(curl_easy_strerror(code));
      }
    private:
      auto_CURL curl_;
      std::string url_;
    };
  }
}
#endif
