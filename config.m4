dnl $Id: config.m4,v 1.6 2005/08/14 16:20:13 koucky Exp $
dnl config.m4 for extension saprfc
dnl don't forget to call PHP_EXTENSION(saprfc)

PHP_ARG_WITH(saprfc, for saprfc support,
[  --with-saprfc[=DIR]     Include saprfc support. DIR is the SAP RFCSDK 
                          install directory.])

if test "$PHP_SAPRFC" != "no"; then
  for i in /usr/sap/rfcsdk /usr/local/rfcsdk /opt/rfcsdk $PHP_SAPRFC; do
    if test -f $i/include/saprfc.h; then
      SAPRFC_DIR=$i
    fi
  done
  
  if test -z "$SAPRFC_DIR"; then
     AC_MSG_ERROR(Please install Non-Unicode SAP RFCSDK 6.20 or 6.40 - I cannot find saprfc.h)
  fi      

  PHP_ADD_INCLUDE($SAPRFC_DIR/include)
  if test "$ext_shared" = "yes"; then
      if test ! -f $SAPRFC_DIR/lib/librfccm.so; then
         AC_MSG_ERROR(Shared RFC Non-Unicode library (file librfccm.so) missing)
      fi   
      PHP_ADD_LIBRARY_WITH_PATH(rfccm, $SAPRFC_DIR/lib, SAPRFC_SHARED_LIBADD)
      PHP_BUILD_SHARED()
  else
      PHP_ADD_LIBRARY_WITH_PATH(rfc, $SAPRFC_DIR/lib, SAPRFC_SHARED_LIBADD)
  fi      
  AC_CANONICAL_HOST
  case "$host" in
    *-hp-*)
        PHP_ADD_LIBRARY_WITH_PATH(cl, $SAPRFC_DIR/lib, SAPRFC_SHARED_LIBADD)
        ;;
  esac	

  PHP_SUBST(SAPRFC_SHARED_LIBADD)
  
  PHP_NEW_EXTENSION(saprfc,saprfc.c rfccal.c,$ext_shared)

  PHP_ADD_BUILD_DIR($SAPRFC_DIR/lib)
fi
