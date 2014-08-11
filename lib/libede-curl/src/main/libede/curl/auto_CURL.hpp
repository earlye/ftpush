#ifndef hCD322AAF_C016_4C60_B6A5_DC2EA86EE628
#define hCD322AAF_C016_4C60_B6A5_DC2EA86EE628

#include <curl/curl.h>

#include <stdexcept>

namespace libede
{
  namespace curl
  {
    class auto_CURL
    {
    public:
      auto_CURL()
        : handle_(init())
      {}

      ~auto_CURL()
      {
        reset(NULL);
      }

      void reset(CURL* value)
      {
        if (value == handle_)
          return;
        if (handle_ != NULL)
          curl_easy_cleanup(handle_);
        handle_ = value;
      }

      CURL* get() const
      {
        return handle_;
      }

      static CURL* init()
      {
        CURL* result = curl_easy_init();
        if ( result == NULL )
          throw std::runtime_error( "failed to get curl_easy handle" );
        return result;
      }

    private:
      CURL* handle_;
    };
  }
}

#endif
