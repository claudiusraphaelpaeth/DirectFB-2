/*
   (c) Copyright 2001-2012  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef ___Direct__TLSObject__H___
#define ___Direct__TLSObject__H___

#ifdef __cplusplus
extern "C" {
#endif

#include <direct/os/thread.h>


#ifdef __cplusplus
}



namespace Direct {


template <typename Object, typename Params, typename Host>
class TLSObject
{
private:
     void      *ctx;
     DirectTLS  tls;

public:
     TLSObject( void *ctx )
          :
          ctx( ctx )
     {
          direct_tls_register( &tls, Host::tls_destroy );
     }

     ~TLSObject()
     {
          direct_tls_unregister( &tls );
     }

     Object *Get( Params &params )
     {
          Object *obj = (Object*) direct_tls_get( tls );

          if (!obj) {
               obj = Host::tls_create( ctx, params );
               if (!obj)
                    return NULL;

               direct_tls_set( tls, obj );
          }

          return obj;
     }

     void Delete()
     {
          Object *obj = (Object*) direct_tls_get( tls );

          if (obj) {
               direct_tls_set( tls, NULL );

               delete obj;
          }
     }
};


template <typename Object, typename Creator=Object, typename Destroyer=Creator>
class TLSObject2
{
private:
     void      *ctx;
     DirectTLS  tls;

public:
     TLSObject2( void *ctx )
          :
          ctx( ctx )
     {
          direct_tls_register( &tls, TLSObject2::destructor );
     }

     ~TLSObject2()
     {
          direct_tls_unregister( &tls );
     }

     Object *Get( void *params )
     {
          Object *obj = (Object*) direct_tls_get( tls );

          if (!obj) {
               obj = Creator::create( ctx, params );
               if (!obj)
                    return NULL;

               direct_tls_set( tls, obj );
          }

          return obj;
     }

     void Delete()
     {
          Object *obj = (Object*) direct_tls_get( tls );

          if (obj) {
               direct_tls_set( tls, NULL );

               delete obj;
          }
     }

private:
     void destructor( void *ptr )
     {
          Destroyer::destroy( ctx, (Object*) ptr );
     }
};



}


}


#endif // __cplusplus

#endif
