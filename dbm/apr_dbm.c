/* ====================================================================
 * The Apache Software License, Version 1.1
 *
 * Copyright (c) 2000-2002 The Apache Software Foundation.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The end-user documentation included with the redistribution,
 *    if any, must include the following acknowledgment:
 *       "This product includes software developed by the
 *        Apache Software Foundation (http://www.apache.org/)."
 *    Alternately, this acknowledgment may appear in the software itself,
 *    if and wherever such third-party acknowledgments normally appear.
 *
 * 4. The names "Apache" and "Apache Software Foundation" must
 *    not be used to endorse or promote products derived from this
 *    software without prior written permission. For written
 *    permission, please contact apache@apache.org.
 *
 * 5. Products derived from this software may not be called "Apache",
 *    nor may "Apache" appear in their name, without prior written
 *    permission of the Apache Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE APACHE SOFTWARE FOUNDATION OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 */

#include "apr.h"
#include "apr_errno.h"
#include "apr_pools.h"
#include "apr_strings.h"
#define APR_WANT_MEMFUNC
#define APR_WANT_STRFUNC
#include "apr_want.h"
#include "apr_general.h"

#include "apu.h"
#include "apu_select_dbm.h"
#include "apr_dbm.h"
#include "apr_dbm_private.h"

/* ### note: the setting of DBM_VTABLE will go away once we have multiple
   ### DBMs in here. 
   ### Well, that day is here.  So, do we remove DBM_VTABLE and the old
   ### API entirely?  Oh, what to do.  We need an APU_DEFAULT_DBM #define.
   ### Sounds like a job for autoconf. */

#if APU_USE_SDBM
#define DBM_VTABLE apr_dbm_type_sdbm
#elif APU_USE_GDBM
#define DBM_VTABLE apr_dbm_type_gdbm
#elif APU_USE_DB
#define DBM_VTABLE apr_dbm_type_db
#else /* Not in the USE_xDBM list above */
#error a DBM implementation was not specified
#endif

APU_DECLARE(apr_status_t) apr_dbm_open_ex(apr_dbm_t **pdb, const char*type, 
                                       const char *pathname, 
                                       apr_int32_t mode, apr_fileperms_t perm,
                                       apr_pool_t *pool)
{
#if APU_HAVE_GDBM
    if (!strcasecmp(type, "GDBM")) {
        return (*apr_dbm_type_gdbm.open)(pdb, pathname, mode, perm, pool);
    }
#endif
#if APU_HAVE_SDBM
    if (!strcasecmp(type, "SDBM")) {
        return (*apr_dbm_type_sdbm.open)(pdb, pathname, mode, perm, pool);
    }
#endif
#if APU_HAVE_DB
    if (!strcasecmp(type, "DB")) {
        return (*apr_dbm_type_db.open)(pdb, pathname, mode, perm, pool);
    }
#endif
    if (!strcasecmp(type, "default")) {
        return (*DBM_VTABLE.open)(pdb, pathname, mode, perm, pool);
    }

    return APR_ENOTIMPL;
} 

APU_DECLARE(apr_status_t) apr_dbm_open(apr_dbm_t **pdb, const char *pathname, 
                                       apr_int32_t mode, apr_fileperms_t perm,
                                       apr_pool_t *pool)
{
    return (*DBM_VTABLE.open)(pdb, pathname, mode, perm, pool);
}

APU_DECLARE(void) apr_dbm_close(apr_dbm_t *dbm)
{
    (*dbm->type->close)(dbm);
}

APU_DECLARE(apr_status_t) apr_dbm_fetch(apr_dbm_t *dbm, apr_datum_t key,
                                        apr_datum_t *pvalue)
{
    return (*dbm->type->fetch)(dbm, key, pvalue);
}

APU_DECLARE(apr_status_t) apr_dbm_store(apr_dbm_t *dbm, apr_datum_t key,
                                        apr_datum_t value)
{
    return (*dbm->type->store)(dbm, key, value);
}

APU_DECLARE(apr_status_t) apr_dbm_delete(apr_dbm_t *dbm, apr_datum_t key)
{
    return (*dbm->type->del)(dbm, key);
}

APU_DECLARE(int) apr_dbm_exists(apr_dbm_t *dbm, apr_datum_t key)
{
    return (*dbm->type->exists)(dbm, key);
}

APU_DECLARE(apr_status_t) apr_dbm_firstkey(apr_dbm_t *dbm, apr_datum_t *pkey)
{
    return (*dbm->type->firstkey)(dbm, pkey);
}

APU_DECLARE(apr_status_t) apr_dbm_nextkey(apr_dbm_t *dbm, apr_datum_t *pkey)
{
    return (*dbm->type->nextkey)(dbm, pkey);
}

APU_DECLARE(void) apr_dbm_freedatum(apr_dbm_t *dbm, apr_datum_t data)
{
    (*dbm->type->freedatum)(dbm, data);
}

APU_DECLARE(char *) apr_dbm_geterror(apr_dbm_t *dbm, int *errcode,
                                     char *errbuf, apr_size_t errbufsize)
{
    if (errcode != NULL)
        *errcode = dbm->errcode;

    /* assert: errbufsize > 0 */

    if (dbm->errmsg == NULL)
        *errbuf = '\0';
    else
        (void) apr_cpystrn(errbuf, dbm->errmsg, errbufsize);
    return errbuf;
}

APU_DECLARE(void) apr_dbm_get_usednames_ex(apr_pool_t *p, 
                                           const char *type, 
                                           const char *pathname,
                                           const char **used1,
                                           const char **used2)
{
#if APU_HAVE_GDBM
    if (!strcasecmp(type, "GDBM")) {
        (*apr_dbm_type_gdbm.getusednames)(p,pathname,used1,used2);
        return;
    }
#endif
#if APU_HAVE_SDBM
    if (!strcasecmp(type, "SDBM")) {
        (*apr_dbm_type_sdbm.getusednames)(p,pathname,used1,used2);
        return;
    }
#endif
#if APU_HAVE_DB
    if (!strcasecmp(type, "DB")) {
        (*apr_dbm_type_db.getusednames)(p,pathname,used1,used2);
        return;
    }
#endif
    if (!strcasecmp(type, "default")) {
        (*DBM_VTABLE.getusednames)(p, pathname, used1, used2);
        return;
    }
} 

APU_DECLARE(void) apr_dbm_get_usednames(apr_pool_t *p,
                                        const char *pathname,
                                        const char **used1,
                                        const char **used2)
{
    /* ### one day, a DBM type name will be passed and we'll need to look it
       ### up. for now, it is constant. */

    (*DBM_VTABLE.getusednames)(p, pathname, used1, used2);
}

/* Most DBM libraries take a POSIX mode for creating files.  Don't trust
 * the mode_t type, some platforms may not support it, int is safe.
 */
APU_DECLARE(int) apr_posix_perms2mode(apr_fileperms_t perm)
{
    int mode = 0;

    mode |= 0700 & (perm >> 2); /* User  is off-by-2 bits */
    mode |= 0070 & (perm >> 1); /* Group is off-by-1 bit */
    mode |= 0007 & (perm);      /* World maps 1 for 1 */
    return mode;
}
