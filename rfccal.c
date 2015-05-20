/* $Id: rfccal.c,v 1.22 2005/08/14 16:20:13 koucky Exp $
 * SAP RFC Client Abstraction Layer (CAL)
 * Author: Eduard Koucky <eduard.koucky@czech-tv.cz> (c) 2001
 */

#include "rfccal.h"

/* #define RFC_MEMORY_DEBUG */
#define TEMP_BUFFER_SIZE 128
#define INIT_ERROR_MESSAGE()   *CAL_LAST_ERROR_MESSAGE=0
#define FEATURE_3x_COMPATIBILITY(version)     version[0]=='3'
#define FEATURE_OPTIONAL_PARAM(version)       (version[0]=='4' && version[1]=='6') || version[0]>'4'
#define FEATURE_UNICODE(version)              version[0]>='6'

static char CAL_LAST_ERROR_MESSAGE[1024];
static char RFC_LAST_ERROR_MESSAGE[1024];
static char RFC_LIBVERSION_INFO[1024];

#ifdef RFC_MEMORY_DEBUG
  static RFC_ENV rfcenv;
#endif

/*********************************************************************
 *     I N T E R N A L   F U N C T I O N S                           *
 *********************************************************************/

/* internal function */

void * DLL_CALL_BACK_FUNCTION cal_allocate (void *old_ptr, unsigned long new_size)
{
    void *ptr=NULL;

    if ( old_ptr == NULL )
    {
        ptr = malloc (new_size);
        zend_printf ("RFC Allocate %lu bytes, ptr %p\n",new_size,ptr);
        return (ptr);
    }
    if ( new_size == 0 )
    {
        if (old_ptr == NULL ) zend_printf ("RFC Free: NULL pointer passed\n");
        else
        {
           free(old_ptr);
           zend_printf ("RFC Free, ptr %p\n",old_ptr);
        }
        return (old_ptr);
    }
    ptr = realloc(old_ptr,new_size);
    zend_printf ("RFC Reallocate %lu bytes, oldptr %p, newptr %p\n",new_size,old_ptr,ptr);
    return ( ptr );
}

/* internal function */

static char *strtoupper (char *s)
{
    char *p;

    p=s;
    if (s)
      while (*p) { *p = toupper (*p); p++; }
    return (s);
}

/* internal function */

static char *strsafecpy (char *dest, char *src, int len)
{
    strncpy (dest,src,len);
    dest[len]=0;
    return (dest);
}


/* internal function, convert RFC value to string */

static char *cal_conv_rfc_to_string (void *rfc_value, CALD_DEFINITION *def)
{
    char *dest,tmp[TEMP_BUFFER_SIZE+1];
    char *p, *s;
    int i;

    dest = (char *) rfc_value;
    memset (tmp,0,TEMP_BUFFER_SIZE+1);
    if (def->retbuf) VFREE(def->retbuf);
    def->retbuf = NULL;

    switch (def->type) {
        case TYPC:     i=def->len-1;
                       if (def->is_rawstr == 0)
                       {
                           while (i>=0 && dest[i] == ' ') i--;
                       }
                       def->retbuf = VCALLOC(i+2);
                       memcpy (def->retbuf,rfc_value,i+1);
                       break;
        case TYPNUM:   i=0;
                       while ((unsigned) i<def->len-1 && dest[i]=='0') i++;
                       def->retbuf = VCALLOC(def->len-i+1);
                       memcpy(def->retbuf,(char *)rfc_value+i,def->len-i);
                       break;
        case TYPX:     def->retbuf = VCALLOC(def->len+1);
                       memcpy (def->retbuf,rfc_value,def->len);
                       break;
        case TYPP:     RfcConvertBcdToChar (rfc_value,def->len,def->decimals,tmp,TEMP_BUFFER_SIZE);
                       p=tmp; while (*p==' ') p++;
                       s=tmp+TEMP_BUFFER_SIZE-1; while (*s==' ') { *s=0; s--; }
                       def->retbuf = VCALLOC (strlen(p)+1);
                       memcpy (def->retbuf,p,strlen(p));
                       break;
        case TYPINT1:  sprintf(tmp,"%hu", *((RFC_INT1 *) rfc_value));
                       def->retbuf = VCALLOC (strlen(tmp)+1);
                       memcpy (def->retbuf,tmp,strlen(tmp));
                       break;
        case TYPINT2:  sprintf(tmp,"%d", *((RFC_INT2 *) rfc_value));
                       def->retbuf = VCALLOC (strlen(tmp)+1);
                       memcpy (def->retbuf,tmp,strlen(tmp));
                       break;
        case TYPINT:   sprintf(tmp,"%d", *((RFC_INT *) rfc_value));
                       def->retbuf = VCALLOC (strlen(tmp)+1);
                       memcpy (def->retbuf,tmp,strlen(tmp));
                       break;
        case TYPFLOAT: sprintf(tmp,"%f", *((RFC_FLOAT *) rfc_value));
                       def->retbuf = VCALLOC (strlen(tmp)+1);
                       memcpy (def->retbuf,tmp,strlen(tmp));
                       break;
        case TYPDATE:
        case TYPTIME:  def->retbuf = VCALLOC (def->len+1);
                       memcpy (def->retbuf,rfc_value,def->len);
                       break;
        default:       def->retbuf = VCALLOC (def->len+1);
                      *def->retbuf = 0;
    }
    return (def->retbuf);
}

/* internal function, convert string to RFC value */

static void cal_conv_string_to_rfc (char *param, void *rfc_value, CALD_DEFINITION *def)
{
    int offset;
    RFC_INT1 *p1;
    RFC_INT2 *p2;
    RFC_INT *pi;
    RFC_FLOAT *pf;


    p1 = rfc_value;
    p2 = rfc_value;
    pi = rfc_value;
    pf = rfc_value;
    if (*param==0)
        switch (def->type) {
             case TYPDATE: param="00000000"; break;
             case TYPTIME: param="000000"; break;
             case TYPNUM:
             case TYPP:
             case TYPINT:
             case TYPINT1:
             case TYPINT2: param="0";break;
             case TYPFLOAT:param="0.0";break;
        }

    switch (def->type) {
       case TYPC:
       case TYPDATE:
       case TYPTIME: memset (rfc_value,' ',def->len);
                     memcpy (rfc_value,param,__MIN(strlen(param),def->len));
                     break;
       case TYPNUM:  memset (rfc_value,'0',def->len);
                     offset=def->len-(__MIN(strlen(param),def->len));
                     memcpy ((char *)rfc_value+offset,param,__MIN(strlen(param),def->len));
                     break;
       case TYPX:    memset (rfc_value,0,def->len);
                     memcpy (rfc_value,param,def->len);
                     break;
       case TYPP:    RfcConvertCharToBcd (param,strlen(param),&def->decimals,rfc_value,def->len);
                     break;
       case TYPINT1: *p1 = (RFC_INT1) atoi(param);
                     break;
       case TYPINT2: *p2 = (RFC_INT2) atoi(param);
                     break;
       case TYPINT : *pi = (RFC_INT) atol(param);
                     break;
       case TYPFLOAT:*pf = atof(param);
                     break;
       default:      memset (rfc_value,0,def->len);   /* unknown data type */
                     break;
    }
}

/* internal function, convert ABAP (C,I,D,T...) type to RFC type */
static void cal_conv_abap_to_rfc (CALD_DEFINITION *def, char abap_type, int length, int decimal)
{

    abap_type = toupper (abap_type);
    switch (abap_type) {
        case 'C': def->type = TYPC;
                  def->len = length;
                  def->decimals = 0;
                  break;
        case 'N': def->type = TYPNUM;
                  def->len = length;
                  def->decimals = 0;
                  break;
        case 'X': def->type = TYPX;
                  def->len = length;
                  def->decimals = 0;
                  break;
        case 'P': def->type = TYPP;
                  def->len = length;
                  def->decimals = decimal;
                  break;
        case 'B': def->type = TYPINT1;
                  def->len = sizeof(RFC_INT1);
                  def->decimals = 0;
                  break;
        case 'S': def->type = TYPINT2;
                  def->len = sizeof(RFC_INT2);
                  def->decimals = 0;
                  break;
        case 'I': def->type = TYPINT;
                  def->len = sizeof(RFC_INT);
                  def->decimals = 0;
                  break;
        case 'F': def->type = TYPFLOAT;
                  def->len = sizeof(RFC_FLOAT);
                  def->decimals = 0;
                  break;
        case 'D': def->type = TYPDATE;
                  def->len = 8;
                  def->decimals = 0;
                  break;
        case 'T': def->type = TYPTIME;
                  def->len = 6;
                  def->decimals = 0;
                  break;
        default : def->type = TYPC;
                  def->len = length;
                  def->decimals = 0;
                  break;
    }
}

/* internal function, RFC type to  ABAP (C,I,D,T...) */
static char cal_conv_rfc_to_abap (int rfc_type)
{
    char abap=' ';

    switch (rfc_type) {
        case TYPC    : abap='C'; break;
        case TYPNUM  : abap='N'; break;
        case TYPX    : abap='X'; break;
        case TYPP    : abap='P'; break;
        case TYPINT1 : abap='b'; break;
        case TYPINT2 : abap='s'; break;
        case TYPINT  : abap='I'; break;
        case TYPFLOAT: abap='F'; break;
        case TYPDATE : abap='D'; break;
        case TYPTIME : abap='T'; break;
        default      : abap=' '; break;
    }
    return (abap);
}

/* internal function,  find existing definition and get offset position in the structure */
CALD_DEFINITION *cal_def_find (CALD_DEFINITION *def, char *item, int *offset)
{
    *offset = 0;
    while (def)
    {
        if ( strcmp (def->item,item)==0 ) break;
        def=def->next;
    }
    if (def) *offset = def->offset;
    return (def);

}


/* internal function, set offset */

CALD_DEFINITION *cal_def_set_offset (CALD_DEFINITION *def, char *item, int p_offset)
{
    int alignment, offset;

    offset = 0;
    while (def)
    {
        switch (def->type) {
           case TYPINT2 :
           case TYPINT  :
           case TYPFLOAT: alignment = offset % def->len; if (alignment) alignment = def->len-alignment; break;
           default      : alignment = 0; break;

        }
        offset += alignment;
        if ( strcmp (def->item,item)==0 ) {
            def->offset =  (p_offset == -1) ? offset : p_offset;
        }
        offset +=  def->len;
        def=def->next;
    }
    return (def);

}

/* internal function, get number of item definitions */
static int cal_def_count (CALD_DEFINITION *def)
{
   int count = 0;
   CALD_DEFINITION *p;

   p=def;
   while (p)
   {
       count++;
       p=p->next;
   }
   return (count);
}

/* internal function, init buffer for value */
static RFC_TYPE_ELEMENT *cal_def_array (CALD_DEFINITION *def,int num)
{
    RFC_TYPE_ELEMENT *defarray;
    int i;

    defarray = VCALLOC(sizeof(RFC_TYPE_ELEMENT)*num);
    if (defarray)
        for (i=0; i<num && def; i++)
        {
           defarray[i].name = def->item;
           defarray[i].type = def->type;
           defarray[i].length = def->len;
           defarray[i].decimals = def->decimals;
           def=def->next;
        }
    return (defarray);
}

/* internal function, create new interface or find existing */
static CALD_INTERFACE *cal_iface_find_or_create (CALD_FUNCTION_MODULE *fce,char *name,int type)
{
    CALD_INTERFACE *iface, *p;

    if (fce->iface == NULL)
      {
         fce->iface = VCALLOC(sizeof(CALD_INTERFACE));
         iface = fce->iface;
      }
    else
      {
         p = iface = fce->iface;
         while (iface)
         {
             if ( iface->type == type && strcmp (iface->name,name)==0 )
                 return (iface);
             p = iface;
             iface = iface->next;
         }
         p->next = VCALLOC(sizeof(CALD_INTERFACE));
         iface = p->next;
      }
    strsafecpy (iface->name,name,CALC_INTERFACE_SZ);
    iface->type = type;
    return (iface);
}

/* internal function,  find existing interface */
static CALD_INTERFACE *cal_iface_find (CALD_FUNCTION_MODULE *fce,char *name,int type)
{
    CALD_INTERFACE *p;

    p = fce->iface;
    while (p)
    {
        if ( p->type==type && strcmp (p->name,name)==0 ) break;
        if ( p->type==CALC_UNDEF && strcmp (p->name,name)==0 ) break;
        p=p->next;
    }
    return (p);
}


/* internal function, add new interface definition */
static void cal_iface_add (CALD_INTERFACE *iface, char* item,char abap, unsigned len,unsigned decimals, int offset)
{
    CALD_DEFINITION *def, *p;
    int alignment;

    if (iface->def == NULL)
      {
         iface->def = VCALLOC(sizeof(CALD_DEFINITION));
         def = iface->def;
         iface->num++;
      }
    else
      {
         p = def = iface->def;
         while (def)
         {
             p = def;
             def = def->next;
         }
         p->next = VCALLOC(sizeof(CALD_DEFINITION));
         def=p->next;
         iface->num++;
      }
    strsafecpy (def->item,item,CALC_DEFINITION_SZ);
    cal_conv_abap_to_rfc (def,abap,len,decimals);
    cal_def_set_offset (iface->def,def->item, offset);

    iface->buflen = def->offset + def->len;
    if (iface->num>1)
        {  alignment = iface->buflen % 8; if (alignment) iface->buflen += 8-alignment; }
}

/* internal function, get count of interface items */
static int cal_iface_count (CALD_FUNCTION_MODULE *fce, int type)
{
   int count = 0;
   CALD_INTERFACE *p;

   p=fce->iface;
   while (p)
   {
       if ( p->type==type || type==CALC_UNDEF ) count++;
       p=p->next;
   }
   return (count);
}

/* internal function, get interface no. index */
static CALD_INTERFACE *cal_iface_get_index (CALD_FUNCTION_MODULE *fce, int type,int index)
{
    CALD_INTERFACE *p;

    p = fce->iface;
    if (type == CALC_UNDEF)
    {
        while (p && index > 0) { index--; p=p->next; }
    }
    else
    {
      while (p)
      {
        if ( p->type == type &&  index == 0) break;
        if ( p->type == type ) index--;
        p=p->next;
      }
    }
    return (p);
}


/* internal function, init buffer for value */
static void cal_iface_init_buffer (CALD_INTERFACE *iface)
{
    CALD_DEFINITION *p;
    int offset;
    char *rawbuf;

    offset = 0;
    p = iface->def;
    while (p)
    {
        cal_def_find (iface->def,p->item, &offset);
        /* special handling for RAW data type inicialization */
        if (p->type == TYPX)
        {
             rawbuf = VCALLOC (p->len);
             if (rawbuf)
             {
                 cal_conv_string_to_rfc (rawbuf,iface->buffer+offset, p);
                 VFREE(rawbuf);
             }
        }
        else
           cal_conv_string_to_rfc ("", iface->buffer+offset, p);
        p = p->next;
    }
    iface->is_set = 0;
}


/*********************************************************************
 *     P U B L I C    F U N C T I O N S                              *
 *********************************************************************/

RFC_HANDLE  _cal_open (char *connect_param)
{
    RFC_ERROR_INFO_EX  error_info;
    RFC_HANDLE rfc;

    INIT_ERROR_MESSAGE();
    rfc = RfcOpenEx (connect_param, &error_info);

    if (rfc == RFC_HANDLE_NULL)
    {
       sprintf(CAL_LAST_ERROR_MESSAGE,"RFC Error Info :\n" );
       sprintf(CAL_LAST_ERROR_MESSAGE+strlen(CAL_LAST_ERROR_MESSAGE),"Group    : %d\n", error_info.group );
       sprintf(CAL_LAST_ERROR_MESSAGE+strlen(CAL_LAST_ERROR_MESSAGE),"Key      : %s\n", error_info.key );
       sprintf(CAL_LAST_ERROR_MESSAGE+strlen(CAL_LAST_ERROR_MESSAGE),"Message  : %s\n", error_info.message );
    }
    return (rfc);
}


CALD_FUNCTION_MODULE *__cal_fce_new (char *name)
{
    CALD_FUNCTION_MODULE *fce;

    INIT_ERROR_MESSAGE();
    fce = VCALLOC (sizeof(CALD_FUNCTION_MODULE));
    strsafecpy (fce->name,name,CALC_FUNCNAME_SZ);
    strsafecpy (fce->rfcsaprl,"40B",3);                 /* set SAP R/3 40B as default version */
    fce->par_offset = -1;   /* set compatibility with  saprfc < 1.1, compute offset position in structure */
    fce->unicode = 0;
    return (fce);
}


int __cal_fce_interface (CALD_FUNCTION_MODULE *fce, int type, char *name, char *item, char abap, unsigned len, unsigned decimals)
{
   CALD_INTERFACE *iface;

   INIT_ERROR_MESSAGE();
   /* check and exit */
   if (!name || !fce) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Null parameter name or fce, __cal_fce_interface()"); return (-1); }
   iface = cal_iface_find_or_create (fce,name,type);
   /* check and exit */  if (!iface) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for iface, __cal_fce_interface()"); return (-1); }
   cal_iface_add (iface,item,abap,len,decimals,fce->par_offset);
   return (0);
}


CALD_INTERFACE_INFO *__cal_fce_getinterface (CALD_FUNCTION_MODULE *fce, char *name, int type)
{
    int size, i,j;
    CALD_INTERFACE_INFO *iinfo;
    CALD_INTERFACE *iface;
    CALD_DEFINITION *p;

    INIT_ERROR_MESSAGE();
    /* check and exit */  if (!fce) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Null parameter fce, __cal_fce_getinterface()"); return (NULL); }
    if (name)
    {
        iface = cal_iface_find (fce,name,type);
        if (!iface) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Interface %s not found, __cal_fce_getinterface()",name); return (NULL); }
        size = 1;
    }
    else
        size = cal_iface_count (fce, CALC_UNDEF);
    iinfo = VCALLOC(sizeof(CALD_INTERFACE_INFO)*(size+1));
    /* check and exit */  if (!iinfo) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for iinfo, __cal_fce_getinterface()"); return (NULL); };
    for (i=0;i<size;i++)
    {
        if (!name)
            iface = cal_iface_get_index (fce,CALC_UNDEF,i);
        iinfo[i].name = iface->name;
        iinfo[i].size = cal_def_count (iface->def);
        iinfo[i].type = iface->type;
        iinfo[i].is_optional = iface->is_optional;
        iinfo[i].buflen = iface->buflen;
        iinfo[i].typeinfo = VCALLOC(sizeof (CALD_INTERFACE_TYPE_INFO)*iinfo[i].size);
        if ( iinfo[i].typeinfo )
        {
           j=0;
           for (p=iface->def;p;p=p->next)
           {
              iinfo[i].typeinfo[j].name  = p->item;
              iinfo[i].typeinfo[j].abap  = cal_conv_rfc_to_abap (p->type);
              iinfo[i].typeinfo[j].length = p->len;
              iinfo[i].typeinfo[j].decimals = p->decimals;
              iinfo[i].typeinfo[j].offset = 0; /* for cal_def_find() */
              cal_def_find (iface->def, p->item, &iinfo[i].typeinfo[j].offset);
              j++;
           }
        }
        else
            sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for iinfo.typeinfo, __cal_fce_getinterface()");
    }
    return (iinfo);
}

int __cal_set (CALD_FUNCTION_MODULE *fce, int type, char *name, char *item, char *value)
{
    CALD_INTERFACE *iface;
    CALD_DEFINITION *def;
    int offset=0;

    INIT_ERROR_MESSAGE();
   /* check and exit */  if (!fce || !name) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Null parameter name or fce, __cal_set()"); return (-1); }
    iface = cal_iface_find (fce,name,type);
    /* check and exit */  if (!iface) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Can't find interface name %s, __cal_set()",name); return (-1); }
    def = cal_def_find (iface->def,item, &offset);
    /* check and exit */  if (!def) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Can't find structure item %s for interface %s, __cal_set()",item,name); return (-1); }
    if ( !iface->buffer )
        iface->buffer = VCALLOC(iface->buflen);
    /* check and exit */  if (!iface->buffer) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for iface->buffer, __cal_set()"); return (-1); }
    cal_conv_string_to_rfc (value, iface->buffer+offset, def);
    if (type != CALC_TABLE)
        iface->is_set = 1;
    return (0);
}

char *__cal_get (CALD_FUNCTION_MODULE *fce, int type, char *name, char *item)
{
    int offset=0;
    CALD_INTERFACE *iface;
    CALD_DEFINITION *def;

    INIT_ERROR_MESSAGE();
    /* check and exit */  if (!fce || !name) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Null parameter name or fce, __cal_get()"); return (NULL); }
    iface = cal_iface_find (fce,name,type);
    /* check and exit */  if (!iface) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Can't find interface name %s, __cal_get()",name); return (NULL); }
    def = cal_def_find (iface->def,item, &offset);
    /* check and exit */  if (!def) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Can't find structure item %s for interface %s, __cal_get()",item,name); return (NULL); }
    if ( !iface->buffer )
    {
        iface->buffer = VCALLOC(iface->buflen);
        if (iface->buffer) cal_iface_init_buffer (iface);
    }
    /* check and exit */  if (!iface->buffer) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for iface->buffer, __cal_get()"); return (NULL); }

    cal_conv_rfc_to_string (iface->buffer+offset, def);
    return (def->retbuf);
}

int __cal_table (CALD_FUNCTION_MODULE *fce, int oper, char *name, int index)
{
    CALD_INTERFACE *iface;
    RFC_RC rfc_rc;
    unsigned len;
    void *ptr;

    INIT_ERROR_MESSAGE();
    /* check and exit */  if (!fce || !name) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Null parameter name or fce, __cal_table()"); return (-1); }
    iface = cal_iface_find (fce,name,CALC_TABLE);
    /* check and exit */  if (!iface) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Can't find interface name %s, __cal_table()",name); return (-1); }
    /* if table isn't passed by RfcGetData, ignore all actions */
    if ( iface->handle == ITAB_NULL && iface->is_rfcgetdata_table == 1) return (0);
    if ( iface->handle == ITAB_NULL )
    {
       iface->defarray = cal_def_array (iface->def,iface->num);
       /* check and exit */ if (!iface->defarray) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for iface->defarray, __cal_table()"); return (-1); }
       rfc_rc = RfcInstallStructure (iface->name,iface->defarray,iface->num,&iface->typehandle);
       /* check and exit */ if (rfc_rc != 0) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: RfcInstallStructure() error #%d, __cal_table()",rfc_rc); return (-1); }
       if (!iface->buffer)
       {
          iface->buffer = VCALLOC(iface->buflen);
          if (iface->buffer) cal_iface_init_buffer (iface);
       }
       /* check and exit */ if (!iface->buffer){ sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for iface->buffer, __cal_table()"); return (-1); }
       iface->handle = ItCreate (iface->name,iface->buflen,0,0);
       /* check and exit */ if (iface->handle == ITAB_NULL) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: ItCreate() error, __cal_table()"); return (-1); }
    }
    switch (oper) {
        case CALC_READ   : len = ItLeng (iface->handle);
                           ptr = ItGetLine (iface->handle,index);
                           /* check and exit */ if (!ptr) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: ItGetLine() error, __cal_table()"); return (-1); }
                           memcpy (iface->buffer,ptr,len);
                           break;
        case CALC_APPEND : len = ItLeng (iface->handle);
                           ptr = ItAppLine (iface->handle);
                           /* check and exit */ if (!ptr) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: ItAppLine() error, __cal_table()"); return (-1); }
                           memcpy (ptr,iface->buffer,len);
                           iface->is_set = 1;
                           break;
        case CALC_INSERT : len = ItLeng (iface->handle);
                           ptr = ItInsLine (iface->handle, index);
                           /* check and exit */ if (!ptr) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: ItInsLine() error, __cal_table()"); return (-1); }
                           memcpy (ptr,iface->buffer,len);
                           iface->is_set = 1;
                           break;
        case CALC_MODIFY : if ( ItPutLine (iface->handle,index,iface->buffer) ) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: ItPutLine() error, __cal_table()") ; return (-1); }
                           iface->is_set = 1;
                           break;
        case CALC_REMOVE : if ( ItDelLine (iface->handle,index) ) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: ItDelLine() error, __cal_table()"); return (-1); }
                           iface->is_set = 1;
                           break;
        case CALC_LENGTH : return ( ItFill (iface->handle) );
        case CALC_INIT   : ItFree (iface->handle);
                           iface->is_set = 0;
                           break;
    }
    return (0);
}

int __call_with_timeout(long timeout, RFC_HANDLE handle, rfc_char_t *function, RFC_PARAMETER *exporting, RFC_PARAMETER *importing, RFC_TABLE *tables, rfc_char_t **exception)
{
    int rfc_rc = RfcCall(handle, function, exporting, tables);
    long ms = 0;
    struct timeval time;
    unsigned long long until;
    unsigned long long now;

    if (rfc_rc == RFC_OK){
        if (timeout >= 0){
            gettimeofday(&time, NULL);
            until = timeout + ((unsigned long long)time.tv_sec * 1000000) + time.tv_usec;
            do
            {
                rfc_rc = RfcListen( handle );
                if( rfc_rc == RFC_RETRY )
                {
                    usleep(100);
                    gettimeofday(&time, NULL);
                    now = ((unsigned long long)time.tv_sec * 1000000) + time.tv_usec;
                }
            } while ( (now < until) && (rfc_rc == RFC_RETRY) );

            if( rfc_rc == RFC_RETRY ){
                rfc_rc = RfcCancel(handle, RFC_CANCEL_CLOSE);
                return PHP_RFC_TIMEOUT_EXPIRED;
            }
        }
        if (rfc_rc == RFC_OK)
            rfc_rc = RfcReceive(handle, importing, tables, exception);

    }else{
        exception = NULL;
    }

    return rfc_rc;
}

int __cal_fce_call (CALD_FUNCTION_MODULE *fce, RFC_HANDLE rfc, long timeout)
{

    RFC_PARAMETER *Importing;
    RFC_PARAMETER *Exporting;
    RFC_TABLE *Tables;
    RFC_RC rfc_rc;
    CALD_INTERFACE *iface;
    int i,j;
    int ret;
    int import_max, export_max, table_max;
    int count;

    INIT_ERROR_MESSAGE();
    /* check and exit */ if (!fce) { sprintf(CAL_LAST_ERROR_MESSAGE, "CALDBG: Null parameter fce, __cal_fce_call()"); return (-1); }

    ret = RFC_OK;
    import_max = cal_iface_count (fce,CALC_IMPORT);
    Importing = VCALLOC((import_max+1)*sizeof(RFC_PARAMETER));

    if (Importing)
    {
       i=0;
       for (j=0;j<import_max;j++)
       {
         iface = cal_iface_get_index (fce,CALC_IMPORT,j);
         if (iface->is_set == 0 && iface->is_optional == 1) continue;
         count = cal_def_count (iface->def);
         if (count == 1)
         {
            Importing[i].name = iface->name;
            Importing[i].nlen = strlen (Importing[i].name);
            Importing[i].type = iface->def->type;
            Importing[i].leng = iface->buflen;
            Importing[i].addr = iface->buffer;
         }
         else
         {
            if (!iface->defarray)
                iface->defarray = cal_def_array (iface->def,iface->num);
            rfc_rc = RfcInstallStructure (iface->name,iface->defarray,iface->num,&iface->typehandle);
            if (rfc_rc)
                { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: RfcInstallStructure() error #%d for %s, __cal_fce_call()",rfc_rc,iface->name); ret = -1; }
            Importing[i].name = iface->name;
            Importing[i].nlen = strlen (Importing[i].name);
            Importing[i].type = iface->typehandle;
            Importing[i].leng = iface->buflen;
            Importing[i].addr = iface->buffer;
         }
         i++;
       }
    }
    else
    {
       sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for Importing, __cal_fce_call()");
       ret = -1;
    }

    export_max = cal_iface_count (fce,CALC_EXPORT);
    Exporting = VCALLOC((export_max+1)*sizeof(RFC_PARAMETER));

    if (Exporting)
    {
       for (i=0;i<export_max;i++)
       {
         iface = cal_iface_get_index (fce,CALC_EXPORT,i);
         count = cal_def_count (iface->def);
         if (count == 1)
         {
            Exporting[i].name = iface->name;
            Exporting[i].nlen = strlen (Exporting[i].name);
            Exporting[i].type = iface->def->type;
            Exporting[i].leng = iface->buflen;
            Exporting[i].addr = iface->buffer;
         }
         else
         {
            if (!iface->defarray)
                iface->defarray = cal_def_array (iface->def,iface->num);
            rfc_rc = RfcInstallStructure (iface->name,iface->defarray,iface->num,&iface->typehandle);
            if (rfc_rc)
                { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: RfcInstallStructure() error #%d for %s, __cal_fce_call()",rfc_rc,iface->name); ret = -1; }
            Exporting[i].name = iface->name;
            Exporting[i].nlen = strlen (Exporting[i].name);
            Exporting[i].type = iface->typehandle;
            Exporting[i].leng = iface->buflen;
            Exporting[i].addr = iface->buffer;
         }
       }
    }
    else
    {
       sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for Exporting, __cal_fce_call()");
       ret = -1;
    }

    table_max = cal_iface_count (fce,CALC_TABLE);
    Tables = VCALLOC((table_max+1)*sizeof(RFC_TABLE));

    if (Tables)
    {
       for (i=0;i<table_max; i++)
       {
          iface = cal_iface_get_index (fce,CALC_TABLE,i);
          Tables[i].name     = iface->name;
          Tables[i].nlen     = strlen (Tables[i].name);
          Tables[i].type     = iface->typehandle;
          Tables[i].ithandle = iface->handle;
          Tables[i].itmode   = RFC_ITMODE_BYREFERENCE;
       }
    }
    else
    {
       sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for Tables, __cal_fce_call()");
       ret = -1;
    }

    if (ret == RFC_OK)
    {
         rfc_rc = __call_with_timeout(timeout, rfc, fce->name, Importing, Exporting, Tables, &fce->exception);
         if (rfc_rc != RFC_OK)
         {
             sprintf(CAL_LAST_ERROR_MESSAGE,"%s",CAL_RFC_LAST_ERROR());
         }
	 else
            fce->exception=NULL;
         ret = rfc_rc;
    }
    if (Importing) VFREE(Importing);
    if (Exporting) VFREE(Exporting);
    if (Tables) VFREE(Tables);

    return (ret);
}

int __cal_fce_indirect_call (CALD_FUNCTION_MODULE *fce, RFC_HANDLE rfc, char *tid)
{

    RFC_PARAMETER *Importing;
    RFC_TABLE *Tables;
    RFC_RC rfc_rc;
    CALD_INTERFACE *iface;
    int i;
    int ret;
    int import_max, table_max;
    int count;

    INIT_ERROR_MESSAGE();
    /* check and exit */ if (!fce) { sprintf(CAL_LAST_ERROR_MESSAGE, "CALDBG: Null parameter fce, __cal_fce_call()"); return (-1); }

    ret = RFC_OK;
    import_max = cal_iface_count (fce,CALC_IMPORT);
    Importing = VCALLOC((import_max+1)*sizeof(RFC_PARAMETER));

    if (Importing)
    {
       for (i=0;i<import_max;i++)
       {
         iface = cal_iface_get_index (fce,CALC_IMPORT,i);
         if (iface->is_set == 0 && iface->is_optional == 1) continue;
         count = cal_def_count (iface->def);
         if (count == 1)
         {
            Importing[i].name = iface->name;
            Importing[i].nlen = strlen (Importing[i].name);
            Importing[i].type = iface->def->type;
            Importing[i].leng = iface->buflen;
            Importing[i].addr = iface->buffer;
         }
         else
         {
            if (!iface->defarray)
                iface->defarray = cal_def_array (iface->def,iface->num);
            rfc_rc = RfcInstallStructure (iface->name,iface->defarray,iface->num,&iface->typehandle);
            if (rfc_rc)
                { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: RfcInstallStructure() error #%d for %s, __cal_fce_call()",rfc_rc,iface->name); ret = -1; }
            Importing[i].name = iface->name;
            Importing[i].nlen = strlen (Importing[i].name);
            Importing[i].type = iface->typehandle;
            Importing[i].leng = iface->buflen;
            Importing[i].addr = iface->buffer;
         }
       }
    }
    else
    {
       sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for Importing, __cal_fce_call()");
       ret = -1;
    }

    table_max = cal_iface_count (fce,CALC_TABLE);
    Tables = VCALLOC((table_max+1)*sizeof(RFC_TABLE));

    if (Tables)
    {
       for (i=0;i<table_max; i++)
       {
          iface = cal_iface_get_index (fce,CALC_TABLE,i);
          Tables[i].name     = iface->name;
          Tables[i].nlen     = strlen (Tables[i].name);
          Tables[i].type     = iface->typehandle;
          Tables[i].ithandle = iface->handle;
		  Tables[i].itmode   = RFC_ITMODE_BYREFERENCE;
       }
    }
    else
    {
       sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for Tables, __cal_fce_call()");
       ret = -1;
    }

    if (ret == RFC_OK)
    {

         rfc_rc = RfcIndirectCallEx (rfc, fce->name, Importing, Tables, tid );
         if (rfc_rc == RFC_OK)
             rfc_rc = RfcConfirmTransID (rfc,tid);
         else
             sprintf(CAL_LAST_ERROR_MESSAGE,"%s",CAL_RFC_LAST_ERROR());
         ret = rfc_rc;

    }
    if (Importing) VFREE(Importing);
    if (Tables) VFREE(Tables);

    return (ret);
}

void __cal_del_fce (CALD_FUNCTION_MODULE *fce)
{
    CALD_INTERFACE *p,*q;
    CALD_DEFINITION *p1, *q1;

    if (!fce) return;
    p = fce->iface;
    while (p)
    {
        q = p->next;
        p1 = p->def;
        while (p1)
        {
            q1 = p1->next;
            if (p1->retbuf) VFREE(p1->retbuf);
            VFREE(p1);
            p1 = q1;
        }
        if (p->defarray) VFREE(p->defarray);
        if (p->buffer) VFREE(p->buffer);
        if (p->handle != ITAB_NULL) ItDelete (p->handle);
        VFREE(p);
        p=q;
    }
    VFREE(fce);
}


void __cal_del_interface (CALD_INTERFACE_INFO *iinfo)
{
   CALD_INTERFACE_INFO *p;

   p = iinfo;
   while (p->name)
   {
     if (p->typeinfo) VFREE(p->typeinfo);
     p++;
   }
   if (iinfo) VFREE(iinfo);
}

void __cal_fce_discover_system_info (CALD_FUNCTION_MODULE *fce, RFC_HANDLE rfc)
{
   CALD_FUNCTION_MODULE *f1;
   int rv;
   char *sapversion;
   char *target_codepage;

   f1 = CAL_NEW("RFC_SYSTEM_INFO");
   /* check and exit */ if (!f1)  return;

   CAL_INTERFACE_EXPORT(f1,"CURRENT_RESOURCES",CAL_DEF_ELEMENT_INT());

 /* Parameters  DIALOG_USER_TYPE and RFC_LOGIN_COMPLETE don't exist
    in interface RFC_SYSTEM_INFO under SAP BASIS 6.40 */

   CAL_INTERFACE_EXPORT(f1,"DIALOG_USER_TYPE",CAL_DEF_ELEMENT_CHAR(1));
   CAL_INTERFACE_EXPORT(f1,"MAXIMAL_RESOURCES",CAL_DEF_ELEMENT_INT());
   CAL_INTERFACE_EXPORT(f1,"RECOMMENDED_DELAY",CAL_DEF_ELEMENT_INT());
   CAL_INTERFACE_EXPORT(f1,"RFC_LOGIN_COMPLETE",CAL_DEF_ELEMENT_CHAR(1));

   CAL_INTERFACE_EXPORT(f1,"RFCSI_EXPORT",CAL_DEF_STRUCTPART_CHAR("RFCPROTO",3));
   CAL_INTERFACE_EXPORT(f1,"RFCSI_EXPORT",CAL_DEF_STRUCTPART_CHAR("RFCCHARTYP",4));
   CAL_INTERFACE_EXPORT(f1,"RFCSI_EXPORT",CAL_DEF_STRUCTPART_CHAR("__PREFIX",104));
   CAL_INTERFACE_EXPORT(f1,"RFCSI_EXPORT",CAL_DEF_STRUCTPART_CHAR("RFCSAPRL",4));
   CAL_INTERFACE_EXPORT(f1,"RFCSI_EXPORT",CAL_DEF_STRUCTPART_CHAR("__POSTFIX",85));


   CAL_SET_EXPORT(f1,"CURRENT_RESOURCES","");
   CAL_SET_EXPORT(f1,"DIALOG_USER_TYPE","");
   CAL_SET_EXPORT(f1,"MAXIMAL_RESOURCES","");
   CAL_SET_EXPORT(f1,"RECOMMENDED_DELAY","");
   CAL_SET_EXPORT(f1,"RFC_LOGIN_COMPLETE","");

   CAL_SET_EXPORT_STRUCT(f1,"RFCSI_EXPORT","RFCPROTO","");
   CAL_SET_EXPORT_STRUCT(f1,"RFCSI_EXPORT","RFCCHARTYP","");
   CAL_SET_EXPORT_STRUCT(f1,"RFCSI_EXPORT","__PREFIX","");
   CAL_SET_EXPORT_STRUCT(f1,"RFCSI_EXPORT","RFCSAPRL","");
   CAL_SET_EXPORT_STRUCT(f1,"RFCSI_EXPORT","__POSTFIX","");

   rv = CAL_CALL(f1,rfc);
   /* check and exit */
   if ( rv ) { CAL_DELETE(f1); return; }

   sapversion = CAL_GET_EXPORT_STRUCT (f1,"RFCSI_EXPORT","RFCSAPRL");
   target_codepage = CAL_GET_EXPORT_STRUCT (f1,"RFCSI_EXPORT","RFCCHARTYP");

   strsafecpy (fce->rfcsaprl,sapversion,strlen(sapversion));
   /* if target SAP system is >= 6.10 and has default code page 4103 or 4102,
      it's UNICODE system */
   if ( target_codepage[0] == '4' &&
        target_codepage[1] == '1' &&
        target_codepage[2] == '0' &&
        ( target_codepage[3] == '2' || target_codepage[3] == '3') &&
        FEATURE_UNICODE(sapversion)
      ) fce->unicode = 1;

   CAL_DELETE(f1);
   return;
}


int __cal_fce_discover_structure (CALD_FUNCTION_MODULE *fce, RFC_HANDLE rfc, int type, char *name, char *structure)
{
   CALD_FUNCTION_MODULE *f1;
   int rv, count, i;
   char *fieldname, *intlength, *decimals, *exid, *offset;
   char abap;
   unsigned len, dec;

   INIT_ERROR_MESSAGE();


   if (fce->unicode == 0) {

    /* On non-unicode SAP system use RFC_GET_STRUCTURE_DEFINITION_P for discovering
      structure */

    f1 = CAL_NEW("RFC_GET_STRUCTURE_DEFINITION_P");
    /* check and exit */ if (!f1) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for f1, __cal_discover_structure()"); return (-1); }

    if ( FEATURE_3x_COMPATIBILITY(fce->rfcsaprl) )
    {
      CAL_INTERFACE_IMPORT(f1,"TABNAME",CAL_DEF_ELEMENT_CHAR(10));
      CAL_INTERFACE_EXPORT(f1,"TABLENGTH",CAL_DEF_ELEMENT_NUM(6));
      CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_CHAR("TABNAME",10));
      CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_CHAR("FIELDNAME",10));
    }
    else
    {
      CAL_INTERFACE_IMPORT(f1,"TABNAME",CAL_DEF_ELEMENT_CHAR(30));
      CAL_INTERFACE_EXPORT(f1,"TABLENGTH",CAL_DEF_ELEMENT_NUM(6));
      CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_CHAR("TABNAME",30));
      CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_CHAR("FIELDNAME",30));
    }
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_NUM("POSITION",4));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_NUM("OFFSET",6));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_NUM("INTLENGTH",6));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_NUM("DECIMALS",6));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_CHAR("EXID",1));

    CAL_SET_IMPORT(f1,"TABNAME",structure);
    CAL_SET_EXPORT(f1,"TABLENGTH","");
    CAL_TBL_INIT(f1,"FIELDS");

    rv = CAL_CALL(f1,rfc);
    /* check and exit */
    if ( rv ) { CAL_DELETE(f1); return (-1); }

    count = CAL_TBL_LENGTH(f1,"FIELDS");
    for (i=1; i<=count; i++)
    {
      if (CAL_TBL_READ(f1,"FIELDS",i) == 0)
      {
          fieldname = CAL_GET_TABLE(f1,"FIELDS","FIELDNAME");
          offset = CAL_GET_TABLE(f1,"FIELDS","OFFSET");
          intlength = CAL_GET_TABLE(f1,"FIELDS","INTLENGTH");
          decimals = CAL_GET_TABLE(f1,"FIELDS","DECIMALS");
          exid = CAL_GET_TABLE(f1,"FIELDS","EXID");

          strtoupper(fieldname);
          abap = *exid;
          len = atoi (intlength);
          dec = atoi (decimals);
          fce->par_offset = atoi(offset);
          __cal_fce_interface (fce,type,name,fieldname,abap,len,dec);
          fce->par_offset = -1;
      }
    }
   } else  {

    /* On unicode SAP system use RFC_GET_UNICODE_STRUCTURE for discovering
      structure */

    f1 = CAL_NEW("RFC_GET_UNICODE_STRUCTURE");
    /* check and exit */ if (!f1) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for f1, __cal_discover_structure()"); return (-1); }

    CAL_INTERFACE_IMPORT(f1,"TABNAME",CAL_DEF_ELEMENT_CHAR(30));
    CAL_INTERFACE_IMPORT(f1,"ALLOW_HALF_DEEP",CAL_DEF_ELEMENT_CHAR(1));
    CAL_INTERFACE_IMPORT(f1,"SKIP_UTF16_INFO",CAL_DEF_ELEMENT_CHAR(1));
    CAL_INTERFACE_IMPORT(f1,"SKIP_UCS_4_INFO",CAL_DEF_ELEMENT_CHAR(1));
    CAL_INTERFACE_EXPORT(f1,"B1_TABLENGTH",CAL_DEF_ELEMENT_INT());
    CAL_INTERFACE_EXPORT(f1,"B2_TABLENGTH",CAL_DEF_ELEMENT_INT());
    CAL_INTERFACE_EXPORT(f1,"B4_TABLENGTH",CAL_DEF_ELEMENT_INT());
    CAL_INTERFACE_EXPORT(f1,"CHAR_LENGTH",CAL_DEF_ELEMENT_INT());
    CAL_INTERFACE_EXPORT(f1,"UUID",CAL_DEF_ELEMENT_RAW(16));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_CHAR("TABNAME",30));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_CHAR("FIELDNAME",30));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_INT("POSITION"));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_CHAR("EXID",1));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_INT("DECIMALS"));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_INT("OFFSET_B1"));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_INT("LENGTH_B1"));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_INT("OFFSET_B2"));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_INT("LENGTH_B2"));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_INT("OFFSET_B4"));
    CAL_INTERFACE_TABLE(f1,"FIELDS",CAL_DEF_STRUCTPART_INT("LENGTH_B4"));

    CAL_SET_IMPORT(f1,"TABNAME",structure);
    CAL_SET_IMPORT(f1,"ALLOW_HALF_DEEP","X");
    CAL_SET_IMPORT(f1,"SKIP_UTF16_INFO","X");
    CAL_SET_IMPORT(f1,"SKIP_UCS_4_INFO","X");
    CAL_SET_EXPORT(f1,"B1_TABLENGTH","0");
    CAL_SET_EXPORT(f1,"B2_TABLENGTH","0");
    CAL_SET_EXPORT(f1,"B4_TABLENGTH","0");
    CAL_SET_EXPORT(f1,"CHAR_LENGTH","0");
    CAL_SET_EXPORT(f1,"UUID","");
    CAL_TBL_INIT(f1,"FIELDS");

    rv = CAL_CALL(f1,rfc);
    /* check and exit */
    if ( rv ) { CAL_DELETE(f1); return (-1); }

    count = CAL_TBL_LENGTH(f1,"FIELDS");
    for (i=1; i<=count; i++)
    {
      if (CAL_TBL_READ(f1,"FIELDS",i) == 0)
      {
          fieldname = CAL_GET_TABLE(f1,"FIELDS","FIELDNAME");
          offset = CAL_GET_TABLE(f1,"FIELDS","OFFSET_B1");
          intlength = CAL_GET_TABLE(f1,"FIELDS","LENGTH_B1");
          decimals = CAL_GET_TABLE(f1,"FIELDS","DECIMALS");
          exid = CAL_GET_TABLE(f1,"FIELDS","EXID");

          strtoupper(fieldname);
          abap = *exid;
          len = atoi (intlength);
          dec = atoi (decimals);
          fce->par_offset = atoi(offset);
          __cal_fce_interface (fce,type,name,fieldname,abap,len,dec);
          fce->par_offset = -1;
      }
    }
   }

   CAL_DELETE(f1);
   return (0);
}

int __cal_fce_discover_interface (CALD_FUNCTION_MODULE *fce, RFC_HANDLE rfc)
{
   CALD_FUNCTION_MODULE *f1;
   CALD_INTERFACE *iface;
   int rv, count, i;
   char *pclass, *parameter, *tabname, *fieldname, *exid, *decimals, *intlength;
   char *optional;
   int type;
   char abap;
   unsigned len, dec;

   INIT_ERROR_MESSAGE();

   __cal_fce_discover_system_info (fce,rfc);
   f1 = CAL_NEW("RFC_GET_FUNCTION_INTERFACE_P");
   /* check and exit */ if (!f1) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for f1, __cal_discover_interface()"); return (-1); }

   CAL_INTERFACE_IMPORT(f1,"FUNCNAME",CAL_DEF_ELEMENT_CHAR(30));
   /* added support for target Unicode system */
   if ( fce->unicode )
          CAL_INTERFACE_IMPORT(f1,"NONE_UNICODE_LENGTH",CAL_DEF_ELEMENT_CHAR(1));

   CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_CHAR("PARAMCLASS",1));
   CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_CHAR("PARAMETER",30));
   if ( FEATURE_3x_COMPATIBILITY(fce->rfcsaprl) )
   {
      CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_CHAR("TABNAME",10));
      CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_CHAR("FIELDNAME",10));
   }
   else
   {
      CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_CHAR("TABNAME",30));
      CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_CHAR("FIELDNAME",30));
   }
   CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_CHAR("EXID",1));
   CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_NUM("POSITION",4));
   CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_NUM("OFFSET",6));
   CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_NUM("INTLENGTH",6));
   CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_NUM("DECIMALS",6));
   CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_CHAR("DEFAULT",21));
   CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_CHAR("PARAMTEXT",79));
   if ( FEATURE_OPTIONAL_PARAM(fce->rfcsaprl) )
        CAL_INTERFACE_TABLE(f1,"PARAMS_P",CAL_DEF_STRUCTPART_CHAR("OPTIONAL",1));  /* in SAP 4.0B unsupported*/

   CAL_SET_IMPORT(f1,"FUNCNAME",fce->name);
   /* added support for target Unicode system */
   if ( fce->unicode )
          CAL_SET_IMPORT(f1,"NONE_UNICODE_LENGTH","X");
   CAL_TBL_INIT(f1,"PARAMS_P");

   rv = CAL_CALL(f1,rfc);
   /* check and exit */
   if ( rv ) { CAL_DELETE(f1); return (-1); }

   count = CAL_TBL_LENGTH(f1,"PARAMS_P");
   for (i=1; i<=count; i++)
   {
      if (CAL_TBL_READ(f1,"PARAMS_P",i) == 0)
      {
          pclass = CAL_GET_TABLE(f1,"PARAMS_P","PARAMCLASS");
          parameter = CAL_GET_TABLE(f1,"PARAMS_P","PARAMETER");
          tabname = CAL_GET_TABLE(f1,"PARAMS_P","TABNAME");
          fieldname = CAL_GET_TABLE(f1,"PARAMS_P","FIELDNAME");
          exid = CAL_GET_TABLE(f1,"PARAMS_P","EXID");
          intlength = CAL_GET_TABLE(f1,"PARAMS_P","INTLENGTH");
          decimals = CAL_GET_TABLE(f1,"PARAMS_P","DECIMALS");
          if ( FEATURE_OPTIONAL_PARAM(fce->rfcsaprl) )
             optional = CAL_GET_TABLE(f1,"PARAMS_P","OPTIONAL");
          else
             optional = "";
          switch (toupper(*pclass))
          {
              case 'I': type = CALC_IMPORT; break;
              case 'E': type = CALC_EXPORT; break;
              case 'T': type = CALC_TABLE; break;
              default : type = CALC_UNDEF;
          }
          strtoupper (parameter);
	  if (*exid == 'h') continue;
	   /*
	       I don't know how handle type 'h' (internal table)
	       with RFC API. Therefore ignore it.
	   */
          else if ( *exid!=0 && *exid!='u')
	  {
              abap = *exid;              /* elementary type */
              len = atoi (intlength);
              dec = atoi (decimals);
              __cal_fce_interface (fce,type,parameter,"",abap,len,dec);
              __cal_set (fce,type,parameter,"","");
              iface = cal_iface_find (fce,parameter,type);
              iface->is_set = 0;
              if ( *optional=='X' ) __cal_fce_optional (fce,type,parameter,1);
          }
          else if (type != CALC_UNDEF)
          {                              /* structure */
              rv = __cal_fce_discover_structure (fce,rfc,type,parameter,tabname);
              /* check and exit */ if (rv) { CAL_DELETE(f1); return (-1); }
              if (type == CALC_TABLE)
                   CAL_TBL_INIT(fce,parameter);
              else
              {
                   iface = cal_iface_find (fce,parameter,type);
                   if ( !iface->buffer )
                       iface->buffer = VCALLOC(iface->buflen);
                   if ( iface->buffer )
                       cal_iface_init_buffer (iface);
              }
          }
      }
   }
   CAL_DELETE(f1);
   return (0);
}

int __cal_fce_optional (CALD_FUNCTION_MODULE *fce, int type, char *name, int opt)
{
    CALD_INTERFACE *iface;

    INIT_ERROR_MESSAGE();
   /* check and exit */  if (!fce || !name) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Null parameter name or fce, __cal_fce_optional()"); return (-1); }
    iface = cal_iface_find (fce,name,type);
    /* check and exit */  if (!iface) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Can't find interface name %s, __cal_fce_optional()",name); return (-1); }
    iface->is_optional = opt;
    return (0);
}

void __cal_fce_init_interface (CALD_FUNCTION_MODULE *fce, int type)
{
   CALD_INTERFACE *iface;

   INIT_ERROR_MESSAGE();
   iface = fce->iface;
   while (iface)
   {
     if (iface->type == type || type == CALC_UNDEF)
     {
       if (iface->type == CALC_TABLE)
          CAL_TBL_INIT(fce,iface->name);
       else
       {
         if ( !iface->buffer )
            iface->buffer = VCALLOC(iface->buflen);
         if ( iface->buffer )
            cal_iface_init_buffer (iface);
       }
     }
     iface = iface->next;
   }
}

int __cal_fce_refresh_internal_buffer (CALD_FUNCTION_MODULE *fce, char *name, int type)
{
   CALD_INTERFACE *iface;
   INIT_ERROR_MESSAGE();

   /* check and exit */  if (!fce || !name) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Null parameter name or fce, __cal_refresh_internal_buffer()"); return (-1); }
   iface = cal_iface_find (fce,name,type);
   /* check and exit */  if (!iface) { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Can't find interface name %s, __cal_refresh_internal_buffer()",name); return (-1); }
   if ( !iface->buffer )
        iface->buffer = VCALLOC(iface->buflen);
   if ( iface->buffer )
        cal_iface_init_buffer (iface);
   else
   { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for buffer, __cal_refresh_internal_buffer()"); return (-1); }

   return (0);

}

char __cal_def_type (CALD_FUNCTION_MODULE *fce, int type, char *name, char *item)
{
    int offset=0;
    CALD_INTERFACE *iface;
    CALD_DEFINITION *def;

    INIT_ERROR_MESSAGE();
    /* check and exit */  if (!fce || !name) return (' ');
    iface = cal_iface_find (fce,name,type);
    /* check and exit */  if (!iface) return (' ');
    def = cal_def_find (iface->def,item, &offset);
    /* check and exit */  if (!def) return (' ');
    return (def->type);

}

int __cal_def_length (CALD_FUNCTION_MODULE *fce, int type, char *name, char *item)
{
    int offset=0;
    CALD_INTERFACE *iface;
    CALD_DEFINITION *def;

    INIT_ERROR_MESSAGE();
   /* check and exit */  if (!fce || !name) return (0);
    iface = cal_iface_find (fce,name,type);
    /* check and exit */  if (!iface) return (0);
    def = cal_def_find (iface->def,item, &offset);
    /* check and exit */  if (!def) return (0);
    return (def->len);

}

char *__cal_get_internal_error_msg()
{
    return (CAL_LAST_ERROR_MESSAGE);
}


char *__cal_last_error ()
{
   RFC_ERROR_INFO error_info;

   *RFC_LAST_ERROR_MESSAGE=0;
   memset( &error_info, 0, sizeof( RFC_ERROR_INFO ) );
   RfcLastError( &error_info );
   sprintf(RFC_LAST_ERROR_MESSAGE,"RFC Error Info :\n" );
   sprintf(RFC_LAST_ERROR_MESSAGE+strlen(RFC_LAST_ERROR_MESSAGE),"Key     : %s\n", error_info.key );
   sprintf(RFC_LAST_ERROR_MESSAGE+strlen(RFC_LAST_ERROR_MESSAGE),"Status  : %s\n", error_info.status );
   sprintf(RFC_LAST_ERROR_MESSAGE+strlen(RFC_LAST_ERROR_MESSAGE),"Message : %s\n", error_info.message );
   sprintf(RFC_LAST_ERROR_MESSAGE+strlen(RFC_LAST_ERROR_MESSAGE),"Internal: %s\n", error_info.intstat );
   return (RFC_LAST_ERROR_MESSAGE);
}

char *__cal_lib_version ()
{
   *RFC_LIBVERSION_INFO=0;

   RfcGetAllLibVersions(RFC_LIBVERSION_INFO,sizeof(RFC_LIBVERSION_INFO));
   return (RFC_LIBVERSION_INFO);
}


void __cal_install_enviroment (void)
{
  #ifdef RFC_MEMORY_DEBUG
     memset (&rfcenv,0,sizeof (RFC_ENV));
     rfcenv.allocate = cal_allocate;
     RfcEnvironment(&rfcenv);
  #endif
}

void __cal_deinstall_enviroment (void)
{
  #ifdef RFC_MEMORY_DEBUG
     memset (&rfcenv,0,sizeof (RFC_ENV));
     RfcEnvironment(&rfcenv);
     RfcClose( RFC_HANDLE_NULL );
  #endif
}

int __sal_get_data (RFC_HANDLE rfc, CALD_FUNCTION_MODULE *fce)
{
    RFC_PARAMETER *Importing;
    RFC_TABLE *Tables;
    RFC_RC rfc_rc;
    CALD_INTERFACE *iface;
    int i;
    int ret;
    int import_max, table_max;
    int count;

    INIT_ERROR_MESSAGE();

    /* check and exit */ if (!fce) { sprintf(CAL_LAST_ERROR_MESSAGE, "CALDBG: Null parameter fce, __sal_get_data()"); return (-1); }

    ret = 0;
    import_max = cal_iface_count (fce,CALC_IMPORT);
    Importing = VCALLOC((import_max+1)*sizeof(RFC_PARAMETER));

    if (Importing)
    {
       for (i=0;i<import_max;i++)
       {
         iface = cal_iface_get_index (fce,CALC_IMPORT,i);
         count = cal_def_count (iface->def);
         if (count == 1)
         {
            Importing[i].name = iface->name;
            Importing[i].nlen = strlen (Importing[i].name);
            Importing[i].type = iface->def->type;
            Importing[i].leng = iface->buflen;
            Importing[i].addr = iface->buffer;
         }
         else
         {
            if (!iface->defarray)
                iface->defarray = cal_def_array (iface->def,iface->num);
            rfc_rc = RfcInstallStructure (iface->name,iface->defarray,iface->num,&iface->typehandle);
            if (rfc_rc)
                { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: RfcInstallStructure() error #%d for %s, __sal_get_data()",rfc_rc,iface->name); ret = -1; }
            Importing[i].name = iface->name;
            Importing[i].nlen = strlen (Importing[i].name);
            Importing[i].type = iface->typehandle;
            Importing[i].leng = iface->buflen;
            Importing[i].addr = iface->buffer;
         }
       }
    }
    else
    {
       sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for Importing, __sal_get_data()");
       ret = -1;
    }

    table_max = cal_iface_count (fce,CALC_TABLE);
    Tables = VCALLOC((table_max+1)*sizeof(RFC_TABLE));

    if (Tables)
    {
       for (i=0;i<table_max; i++)
       {
          /* Drop itab handle, get handle from RfcGetData() */
          iface = cal_iface_get_index (fce,CALC_TABLE,i);
          if (iface->handle != ITAB_NULL)
          {
              ItFree (iface->handle);
              ItDelete (iface->handle);
              iface->handle = ITAB_NULL;
          }
          Tables[i].name     = iface->name;
          Tables[i].nlen     = strlen (Tables[i].name);
          Tables[i].type     = iface->typehandle;
          Tables[i].ithandle = ITAB_NULL;
          Tables[i].itmode   = RFC_ITMODE_BYREFERENCE;
       }
    }
    else
    {
       sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for Tables, __sal_get_data()");
       ret = -1;
    }

    if (ret == 0)
    {
         rfc_rc = RfcGetData (rfc, Importing, Tables);
         if (rfc_rc != RFC_OK)
            sprintf(CAL_LAST_ERROR_MESSAGE,"%s",CAL_RFC_LAST_ERROR());
         else
         {
            for (i=0;i<table_max; i++)
            {
               /* Set new itab handle from RfcGetData() */
               iface = cal_iface_get_index (fce,CALC_TABLE,i);
               iface->handle = Tables[i].ithandle;
               /* iface->handle can be null, if internal table is not passed by caller */
               /* don't create handle in __cal_table(), because this is memory leak */
               iface->is_rfcgetdata_table = 1;
            }
         }
         ret = rfc_rc;
    }
    if (Importing) VFREE(Importing);
    if (Tables) VFREE(Tables);

    return (ret);
}


int __sal_send_data (RFC_HANDLE rfc, CALD_FUNCTION_MODULE *fce)
{
    RFC_PARAMETER *Exporting;
    RFC_TABLE *Tables;
    RFC_RC rfc_rc;
    CALD_INTERFACE *iface;
    int i,j;
    int ret;
    int export_max, table_max;
    int count;

    INIT_ERROR_MESSAGE();

    /* check and exit */ if (!fce) { sprintf(CAL_LAST_ERROR_MESSAGE, "CALDBG: Null parameter fce, __sal_send_data()"); return (-1); }

    ret = 0;
    export_max = cal_iface_count (fce,CALC_EXPORT);
    Exporting = VCALLOC((export_max+1)*sizeof(RFC_PARAMETER));

    if (Exporting)
    {
       for (i=0;i<export_max;i++)
       {
         iface = cal_iface_get_index (fce,CALC_EXPORT,i);
         count = cal_def_count (iface->def);
         if (count == 1)
         {
            Exporting[i].name = iface->name;
            Exporting[i].nlen = strlen (Exporting[i].name);
            Exporting[i].type = iface->def->type;
            Exporting[i].leng = iface->buflen;
            Exporting[i].addr = iface->buffer;
         }
         else
         {
            if (!iface->defarray)
                iface->defarray = cal_def_array (iface->def,iface->num);
            rfc_rc = RfcInstallStructure (iface->name,iface->defarray,iface->num,&iface->typehandle);
            if (rfc_rc)
                { sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: RfcInstallStructure() error #%d for %s, __sal_send_data()",rfc_rc,iface->name); ret = -1; }
            Exporting[i].name = iface->name;
            Exporting[i].nlen = strlen (Exporting[i].name);
            Exporting[i].type = iface->typehandle;
            Exporting[i].leng = iface->buflen;
            Exporting[i].addr = iface->buffer;
         }
       }
    }
    else
    {
       sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for Exporting, __sal_send_data()");
       ret = -1;
    }

    table_max = cal_iface_count (fce,CALC_TABLE);
    Tables = VCALLOC((table_max+1)*sizeof(RFC_TABLE));

    if (Tables)
    {
       j=0;
       for (i=0;i<table_max; i++)
       {
          iface = cal_iface_get_index (fce,CALC_TABLE,i);
          if (iface->handle != ITAB_NULL )
          {
             Tables[j].name     = iface->name;
             Tables[j].nlen     = strlen (Tables[j].name);
             Tables[j].type     = iface->typehandle;
             Tables[j].ithandle = iface->handle;
             Tables[j].itmode   = RFC_ITMODE_BYREFERENCE;
             j++;
          }
       }
    }
    else
    {
       sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for Tables, __sal_send_data()");
       ret = -1;
    }

    if (ret == 0)
    {
         rfc_rc = RfcSendData (rfc, Exporting, Tables);
         if (rfc_rc != RFC_OK)
             sprintf(CAL_LAST_ERROR_MESSAGE,"%s",CAL_RFC_LAST_ERROR());
         ret = rfc_rc;
    }
    if (Exporting) VFREE(Exporting);
    if (Tables) VFREE(Tables);

    return (ret);
}


int __sal_raise (RFC_HANDLE rfc, CALD_FUNCTION_MODULE *fce, char *exception)
{
    CALD_INTERFACE *iface;
    int i;
    int ret;
    RFC_TABLE *Tables;
    int table_max;

    INIT_ERROR_MESSAGE();

    /* check and exit */ if (!fce || !exception) { sprintf(CAL_LAST_ERROR_MESSAGE, "CALDBG: Null parameter fce, __sal_send_data()"); return (-1); }

    ret = 0;
    table_max = cal_iface_count (fce,CALC_TABLE);
    if (table_max == 0)
    {
       ret = RfcRaise (rfc,exception);
    }
    else
    {
       Tables = VCALLOC((table_max+1)*sizeof(RFC_TABLE));

       if (Tables)
       {
           for (i=0;i<table_max; i++)
           {
               iface = cal_iface_get_index (fce,CALC_TABLE,i);
               Tables[i].name     = iface->name;
               Tables[i].nlen     = strlen (Tables[i].name);
               Tables[i].type     = iface->typehandle;
               Tables[i].ithandle = iface->handle;
           }
           ret = RfcRaiseTables (rfc,exception,Tables);
	   VFREE(Tables);
        }
        else
        {
           sprintf(CAL_LAST_ERROR_MESSAGE,"CALDBG: Memory allocation error for Tables, __sal_raise()");
           ret = -1;
        }
    }
    return (ret);
}

/*  set is_rawstr = 1 for TYPC parameters of interface */
void __cal_set_rawstr (CALD_FUNCTION_MODULE *fce)
{
    CALD_DEFINITION *p;
    CALD_INTERFACE *iface;

    iface = fce->iface;
    while (iface)
    {
      p = iface->def;
      while (p)
      {
        if (p->type == TYPC) p->is_rawstr = 1;
        p = p->next;
      }
      iface = iface->next;
    }
}
