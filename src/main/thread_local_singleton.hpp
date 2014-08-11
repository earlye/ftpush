#ifndef hF9DAD5B3_3D75_425F_B922_95DE3012C581
#define hF9DAD5B3_3D75_425F_B922_95DE3012C581

// TODO: recover thread_local_singleton from svn...
#include <stack>
namespace ecpp
{
  template< typename OBJECT_TYPE , typename TAG_TYPE >
  class thread_local_singleton
  {
  public:
    typedef OBJECT_TYPE object_type;
    typedef TAG_TYPE tag_type;
    static object_type& instance()
    {
      static object_type result;
      return result;
    }
  };
}

#endif
