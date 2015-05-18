<?
// ----------------------------------------------------------------------------
// SAPRFC - Client example
// Call function module RFC_READ_REPORT - Get ABAP report from SAP R/3
// http://saprfc.sourceforge.net
// ----------------------------------------------------------------------------
//----------------------------------------------------------------------
//RFC_READ_REPORT:
//       IMPORTING
//             VALUE(PROGRAM) LIKE  SY-REPID
//       EXPORTING
//             VALUE(SYSTEM) LIKE  SY-SYSID
//             VALUE(TRDIR) LIKE  TRDIR STRUCTURE  TRDIR
//       TABLES
//              QTAB STRUCTURE  D022S
//----------------------------------------------------------------------


$REPORT = "RSUSR000";                            // Set name of the report

$LOGIN = array (                                 // Set login data to R/3
			"ASHOST"=>"garfield",                // application server host name
			"SYSNR"=>"30",                       // system number
			"CLIENT"=>"900",                     // client
			"USER"=>"rfctest",                   // user
			"PASSWD"=>"*****",                   // password
			"CODEPAGE"=>"1404");                 // codepage
   
// ----------------------------------------------------------------------------

   $rfc = saprfc_open ($LOGIN);
   if (! $rfc )
   {
       echo "RFC connection failed with error:".saprfc_error();
       exit;
   }

   $fce = saprfc_function_discover($rfc, "RFC_READ_REPORT");
   if (! $fce )
   {
       echo "Discovering interface of function module RFC_READ_REPORT failed";
       exit;
   }

   saprfc_import ($fce,"PROGRAM",$REPORT);
   saprfc_table_init ($fce,"QTAB");

   $rc = saprfc_call_and_receive ($fce);
   if ($rfc_rc != SAPRFC_OK)
   {
       if ($rfc == SAPRFC_EXCEPTION )
           echo ("Exception raised: ".saprfc_exception($fce));
       else
           echo ("Call error: ".saprfc_error($fce));
       exit;
   }
   
   $SYSTEM = saprfc_export ($fce,"SYSTEM");
   $TRDIR = saprfc_export ($fce,"TRDIR");
   $rows = saprfc_table_rows ($fce,"QTAB");
   echo "<PRE>";
   for ($i=1; $i<=$rows; $i++)
   {
	 $QTAB = saprfc_table_read ($fce,"QTAB",$i);
     echo ($QTAB[LINE]."\n");
   }
   echo "</PRE>";
   saprfc_function_free($fce);
   saprfc_close($rfc);
?>

