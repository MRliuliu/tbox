/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2015, ruki All rights reserved.
 *
 * @author      ruki
 * @file        ifaddrs.c
 * @ingroup     platform
 */

/* //////////////////////////////////////////////////////////////////////////////////////
 * trace
 */
#define TB_TRACE_MODULE_NAME                "ifaddrs"
#define TB_TRACE_MODULE_DEBUG               (1)

/* //////////////////////////////////////////////////////////////////////////////////////
 * includes
 */
#include "ifaddrs.h"
#include "../utils/utils.h"
#include "../algorithm/algorithm.h"

/* //////////////////////////////////////////////////////////////////////////////////////
 * instance implementation
 */
static tb_handle_t tb_ifaddrs_instance_init(tb_cpointer_t* ppriv)
{
    // init it
    return tb_ifaddrs_init();
}
static tb_void_t tb_ifaddrs_instance_exit(tb_handle_t ifaddrs, tb_cpointer_t priv)
{
    // exit it
    tb_ifaddrs_exit((tb_ifaddrs_ref_t)ifaddrs);
}
static tb_long_t tb_ifaddrs_interface_comp(tb_iterator_ref_t iterator, tb_cpointer_t item, tb_cpointer_t name)
{
    // check
    tb_assert_return_val(item, 0);

    // comp
    return tb_stricmp(((tb_ifaddrs_interface_ref_t)item)->name, (tb_char_t const*)name);
}
static tb_ifaddrs_interface_ref_t tb_ifaddrs_interface_find(tb_iterator_ref_t iterator, tb_char_t const* name)
{
    // check
    tb_assert_and_check_return_val(iterator && name, tb_null);

    // find it
    tb_size_t itor = tb_find_all_if(iterator, tb_ifaddrs_interface_comp, name);
    tb_check_return_val(itor != tb_iterator_tail(iterator), tb_null);

    // ok
    return (tb_ifaddrs_interface_ref_t)tb_iterator_item(iterator, itor);
}

/* //////////////////////////////////////////////////////////////////////////////////////
 * implementation
 */
#ifdef TB_CONFIG_OS_WINDOWS
//#   include "windows/ifaddrs.c"
#elif defined(TB_CONFIG_OS_LINUX)
//#   include "linux/ifaddrs.c"
#elif defined(TB_CONFIG_OS_ANDROID)
//#   include "linux/android/ifaddrs.c"
#elif defined(TB_CONFIG_API_HAVE_POSIX)
#   include "posix/ifaddrs.c"
#else
tb_ifaddrs_ref_t tb_ifaddrs_init()
{
    tb_trace_noimpl();
    return tb_null;
}
tb_void_t tb_ifaddrs_exit(tb_ifaddrs_ref_t ifaddrs)
{
    tb_trace_noimpl();
}
tb_iterator_ref_t tb_ifaddrs_itor(tb_ifaddrs_ref_t ifaddrs, tb_bool_t reload)
{
    tb_trace_noimpl();
    return tb_null;
}
#endif
tb_ifaddrs_ref_t tb_ifaddrs()
{
    return (tb_ifaddrs_ref_t)tb_singleton_instance(TB_SINGLETON_TYPE_IFADDRS, tb_ifaddrs_instance_init, tb_ifaddrs_instance_exit, tb_null);
}
tb_ifaddrs_interface_ref_t tb_ifaddrs_interface(tb_ifaddrs_ref_t ifaddrs, tb_char_t const* name, tb_bool_t reload)
{
    // check
    tb_assert_and_check_return_val(ifaddrs && name, tb_null);

    // the iterator
    tb_iterator_ref_t iterator = tb_ifaddrs_itor(ifaddrs, reload);
    tb_assert_and_check_return_val(iterator, tb_null);

    // reload it if the cached interfaces is empty
    if (!reload && !tb_iterator_size(iterator)) iterator = tb_ifaddrs_itor(ifaddrs, tb_true);

    // ok
    return tb_ifaddrs_interface_find(iterator, name);
}
tb_bool_t tb_ifaddrs_hwaddr(tb_ifaddrs_ref_t ifaddrs, tb_char_t const* name, tb_bool_t reload, tb_hwaddr_ref_t hwaddr)
{
    // check
    tb_assert_and_check_return_val(ifaddrs && hwaddr, tb_false);

    // clear it first
    tb_hwaddr_clear(hwaddr);

    // the iterator
    tb_iterator_ref_t iterator = tb_ifaddrs_itor(ifaddrs, reload);
    tb_assert_and_check_return_val(iterator, tb_false);

    // reload it if the cached interfaces is empty
    if (!reload && !tb_iterator_size(iterator)) iterator = tb_ifaddrs_itor(ifaddrs, tb_true);

    // done
    tb_bool_t ok = tb_false;
    tb_for_all_if (tb_ifaddrs_interface_ref_t, interface, iterator, interface)
    {
        // get hwaddr from the given interface name?
        if (name)
        {
            // is this?
            if (    (interface->flags & TB_IFADDRS_INTERFACE_FLAG_HAVE_HWADDR)
                &&  !(interface->flags & TB_IFADDRS_INTERFACE_FLAG_IS_LOOPBACK)
                &&  (interface->name && !tb_strcmp(interface->name, name)))
            {
                // save hwaddr
                tb_hwaddr_copy(hwaddr, &interface->hwaddr);

                // ok
                ok = tb_true;
                break;
            }
        }
        else
        {
            // is this?
            if (    (interface->flags & TB_IFADDRS_INTERFACE_FLAG_HAVE_HWADDR)
                &&  (interface->flags & TB_IFADDRS_INTERFACE_FLAG_HAVE_IPADDR)
                &&  !(interface->flags & TB_IFADDRS_INTERFACE_FLAG_IS_LOOPBACK))
            {
                // save hwaddr
                tb_hwaddr_copy(hwaddr, &interface->hwaddr);

                // ok
                ok = tb_true;
                break;
            }
        }
    }

    // ok?
    return ok;
}
tb_bool_t tb_ifaddrs_ipaddr(tb_ifaddrs_ref_t ifaddrs, tb_char_t const* name, tb_bool_t reload, tb_size_t family, tb_ipaddr_ref_t ipaddr)
{
    // check
    tb_assert_and_check_return_val(ifaddrs && ipaddr, tb_false);

    // clear it first
    tb_ipaddr_clear(ipaddr);

    // the iterator
    tb_iterator_ref_t iterator = tb_ifaddrs_itor(ifaddrs, reload);
    tb_assert_and_check_return_val(iterator, tb_false);

    // reload it if the cached interfaces is empty
    if (!reload && !tb_iterator_size(iterator)) iterator = tb_ifaddrs_itor(ifaddrs, tb_true);

    // the ipaddr flags
    tb_uint32_t ipflags = 0;
    if (family == TB_IPADDR_FAMILY_IPV4) ipflags |= TB_IFADDRS_INTERFACE_FLAG_HAVE_IPADDR4;
    else if (family == TB_IPADDR_FAMILY_IPV6) ipflags |= TB_IFADDRS_INTERFACE_FLAG_HAVE_IPADDR6;

    // done
    tb_bool_t ok = tb_false;
    tb_for_all_if (tb_ifaddrs_interface_ref_t, interface, iterator, interface)
    {
        // is this?
        if (    !(interface->flags & TB_IFADDRS_INTERFACE_FLAG_IS_LOOPBACK)
            &&  (interface->flags & TB_IFADDRS_INTERFACE_FLAG_HAVE_IPADDR)
            &&  (!name || (interface->name && !tb_strcmp(interface->name, name))))
        {
            // ipv4?
            if (    interface->flags & TB_IFADDRS_INTERFACE_FLAG_HAVE_IPADDR4
                &&  (!family || family == TB_IPADDR_FAMILY_IPV4))
            {
                // save ipaddr4
                tb_ipaddr_ipv4_set(ipaddr, &interface->ipaddr4);

                // ok
                ok = tb_true;
                break;
            }
            // ipv6?
            else if (    interface->flags & TB_IFADDRS_INTERFACE_FLAG_HAVE_IPADDR6
                    &&  (!family || family == TB_IPADDR_FAMILY_IPV6))
            {
                // save ipaddr6
                tb_ipaddr_ipv6_set(ipaddr, &interface->ipaddr6);

                // ok
                ok = tb_true;
                break;
            }
        }
    }

    // ok?
    return ok;
}
#ifdef __tb_debug__
tb_void_t tb_ifaddrs_dump(tb_ifaddrs_ref_t ifaddrs)
{
    // trace
    tb_trace_i("");

    // done
    tb_for_all_if (tb_ifaddrs_interface_ref_t, interface, tb_ifaddrs_itor(ifaddrs, tb_true), interface)
    {
        // trace
        tb_trace_i("name: %s%s", interface->name, (interface->flags & TB_IFADDRS_INTERFACE_FLAG_IS_LOOPBACK)? "[loopback]" : "");
        if (interface->flags & TB_IFADDRS_INTERFACE_FLAG_HAVE_IPADDR4)
            tb_trace_i("    ipaddr4: %{ipv4}",  &interface->ipaddr4);
        if (interface->flags & TB_IFADDRS_INTERFACE_FLAG_HAVE_IPADDR6)
            tb_trace_i("    ipaddr6: %{ipv6}",  &interface->ipaddr6);
        if (interface->flags & TB_IFADDRS_INTERFACE_FLAG_HAVE_HWADDR)
            tb_trace_i("    hwaddr: %{hwaddr}", &interface->hwaddr);
    }
}
#endif
