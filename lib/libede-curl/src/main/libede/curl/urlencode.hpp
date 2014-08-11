#ifndef hD0CF3B76_20C1_45FB_A474_0A2918C7E2F0
#define hD0CF3B76_20C1_45FB_A474_0A2918C7E2F0

namespace libede
{
  namespace curl
  {
    std::string urlencode( std::string const& string )
    {
      static auto_CURL handle;
      char* escaped = curl_easy_escape(handle.get(), string.c_str(), string.length());
      std::string result(escaped);
      curl_free(escaped);
      return result;
    }
  }
}

#endif
