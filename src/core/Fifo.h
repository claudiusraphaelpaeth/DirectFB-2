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

#ifndef ___DirectFB__Fifo__H___
#define ___DirectFB__Fifo__H___


#ifdef __cplusplus
extern "C" {
#endif

#include <direct/fifo.h>
#include <direct/os/mutex.h>
#include <direct/os/waitqueue.h>



#ifdef __cplusplus
}


#include <queue>


namespace DirectFB {


#define DFB_FIFO_WAIT_SUPPORT (1)


template <typename T>
class FIFO
{
public:
     FIFO()
          :
          num_items( 0 )
     {
          direct_mutex_init( &lock );
          direct_waitqueue_init( &wq );
#if DFB_FIFO_WAIT_SUPPORT
          direct_waitqueue_init( &wq_empty );
#endif
     }

     ~FIFO()
     {
          direct_mutex_deinit( &lock );
          direct_waitqueue_deinit( &wq );
#if DFB_FIFO_WAIT_SUPPORT
          direct_waitqueue_deinit( &wq_empty );
#endif
     }

     void
     push( T e )
     {
          direct_mutex_lock( &lock );

          list.push( e );
          num_items++;

          direct_waitqueue_signal( &wq );

          direct_mutex_unlock( &lock );
     }

     T
     pull()
     {
          T e;

          direct_mutex_lock( &lock );

          while (list.empty())
               direct_waitqueue_wait( &wq, &lock );

          e = list.front();
          list.pop();
//          list.erase( list.begin() );
          num_items--;

#if DFB_FIFO_WAIT_SUPPORT
//          if (list.empty())
               direct_waitqueue_broadcast( &wq_empty );
#endif

          direct_mutex_unlock( &lock );

          return e;
     }

     bool
     empty()
     {
          bool val;

          direct_mutex_lock( &lock );

          val = list.empty();

          direct_mutex_unlock( &lock );

          return val;
     }

     void
     waitEmpty()
     {
          direct_mutex_lock( &lock );

#if DFB_FIFO_WAIT_SUPPORT
          while (!list.empty())
               direct_waitqueue_wait( &wq_empty, &lock );
#endif

          direct_mutex_unlock( &lock );
     }

     void
     waitMost( size_t count )
     {
          direct_mutex_lock( &lock );

#if DFB_FIFO_WAIT_SUPPORT
          while (num_items > count)
               direct_waitqueue_wait( &wq_empty, &lock );
#endif

          direct_mutex_unlock( &lock );
     }

     size_t
     count()
     {
          return num_items;
     }

private:
     DirectMutex     lock;
     DirectWaitQueue wq;
#if DFB_FIFO_WAIT_SUPPORT
     DirectWaitQueue wq_empty;
#endif

     std::queue<T>   list;
     size_t          num_items;
};


template <typename T>
class FastFIFO
{
class Element {
public:
     DirectFifoItem item;
     T              val;
};

public:
     FastFIFO()
     {
          direct_fifo_init( &fifo );
     }

     ~FastFIFO()
     {
          direct_fifo_destroy( &fifo );
     }

     void
     push( T e )
     {
          Element *element = new Element;

          element->item.magic = 0;
          element->val        = e;

          direct_fifo_push( &fifo, &element->item );
     }

     T
     pull()
     {
          Element *element;
          T        val;

          do {
               element = (Element*) direct_fifo_pull( &fifo );
               if (!element)
                    direct_fifo_wait( &fifo );
          } while (!element);

          val = element->val;

          delete element;

          return val;
     }

private:
     DirectFifo      fifo;
};


}


#endif // __cplusplus

#endif
