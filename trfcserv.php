#!/usr/local/bin/php -q
<?
// ----------------------------------------------------------------------------
// trfcserv.php - rewritten from C example trfcserv.c (in the SAP RFCSDK)
// Require: CGI version of PHP
// http://saprfc.sourceforge.net
// ----------------------------------------------------------------------------

/*====================================================================*/
/*                                                                    */
/*     PROGRAM     :   trfcserv.c (running on Windows_NT, Windows_95, */
/*                                 OS/2 and R/3-platforms)            */
/*                                                                    */
/*                                                                    */
/*     DESCRIPTION :   Sample Server-Program for transactional RFC    */
/*                     Following function is available:               */
/*                                                                    */
/*                       - STFC_WRITE_TO_TCPIC                        */
/*                                                                    */
/*                     and can only be called from R/3 >= 3.0         */
/*                                                                    */
/*                                                                    */
/*                     trfcserv.tid:  TID-Management                  */
/*                     trfcserv.lck:  Lock file for access TID-Mgm.   */
/*                     trnn...n.dat:  Received data from R/3-ABAP     */
/*                     trnn...n.trc:  Trace file for RFC-Protocol     */
/*                                    (nn...n: Process ID)            */
/*                                                                    */
/*                     These files will be created/appended in the    */
/*                     directory defined via TRFC_WORK_DIR. If this   */
/*                     Environment Variable is not defined these files*/
/*                     will be in the working directory of the user   */
/*                     who started this program (mostly user from     */
/*                     SAP Gateway).                                  */
/*                                                                    */
/*                     Trace files will only be written if the        */
/*                     Environment Variable TRFC_TRACE is defined and */
/*                     not equal 0.                                   */
/*                                                                    */
/*                     On UNIX this program can be started via a      */
/*                     sript with setting the required Environment    */
/*                     Variable as follow:                            */
/*                                                                    */
/*                     transrfc:                                      */
/*                     #!/bin/csh                                     */
/*                     setenv TRFC_WORK_DIR /usr/sap/rfctest          */
/*                     setenv TRFC_TRACE    1                         */
/*                     /usr/sap/rfctest/trfcserv $*                   */
/*                                                                    */
/*                     Please do the same for other platforms.        */
/*                                                                    */
/*                     The entry in sm59 must contain the name of     */
/*                     this script (transrfc) as name of program.     */
/*                                                                    */
/*                     Because of working with file system it is      */
/*                     recommended to choose the working directory as */
/*                     a local directory and NOT mounted via NFS.     */
/*                                                                    */
/*                                                                    */
/*     SAP AG Walldorf                                                */
/*     Systeme, Anwendungen und Produkte in der Datenverarbeitung     */
/*                                                                    */
/*     Copyright (C) SAP AG 1995                                      */
/*                                                                    */
/*====================================================================*/

/*====================================================================*/
/*  Function:  main                                                   */
/*                                                                    */
/*    Accept RFC-Connection                                           */
/*    Install all offering functions                                  */
/*    Loop in waiting for Remote Function Call                        */
/*      do function                                                   */
/*    until client disconnects the RFC-Connection                     */
/*                                                                    */
/*====================================================================*/

function main($argc, $argv)
{
  global $rfc_handle, $tid_file, $lock_file, $datetime, $trace_fp, $stderr;
  global $working_dir;

  $stderr = fopen ("php://stderr","a");

  if ($argc == 1)
  {
    help();
    exit();
  }

  /*------------------------------------------------------------------*/
  /* Output argument list                                             */
  /*------------------------------------------------------------------*/

  fputs ($stderr,sprintf("\nargc    = %d", $argc));
  for ($i=0; $i<$argc; $i++)
   fputs ($stderr,sprintf("\nargv[%d] = '%s'", $i, $argv[$i]));
  fflush($stderr);

  /*------------------------------------------------------------------*/
  /* Get working directory and names of TID- and LOCK-file            */
  /*------------------------------------------------------------------*/
  $working_dir = getenv("TRFC_WORK_DIR");
  if (! empty ($working_dir) ) $working_dir.="/";
  $tid_file = $working_dir."trfcserv.tid";
  $lock_file = $working_dir."trfcserv.lck";

  if (getenv ("TRFC_TRACE") != "" && getenv ("TRFC_TRACE") != "0")
  {
      create_file_name ($trace_file);
      $trace_file.=".trc";
      $trace_fp = fopen ($trace_file,"a");
  }
  else
      $trace_fp = false;

  $tbuf = "\n********************************\n";
  $datetime = date("D M j G:i:s Y");
  $tbuf .= sprintf("*   %s   *\n", $datetime);
  $tbuf .= "********************************\n";
  TRFC_trace($tbuf);

  /*------------------------------------------------------------------*/
  /* Initialize the TID management                                    */
  /*------------------------------------------------------------------*/
  init_TID();

  /*------------------------------------------------------------------*/
  /* Accept RFC-Connection                                            */
  /*------------------------------------------------------------------*/
  $tbuf = "\n<==  RfcAccept                           rfc_handle = ";
  TRFC_trace($tbuf);

  $rfc_handle = saprfc_server_accept ($argv);

  if ($rfc_handle == false)
  {
    $tbuf = "RFC_HANDLE_NULL";
    TRFC_trace($tbuf);
    rfc_error("RfcAccept");
  }
  $tbuf = sprintf("%u", $rfc_handle);
  TRFC_trace($tbuf);

  /*------------------------------------------------------------------*/
  /* Install offering functions                                       */
  /*------------------------------------------------------------------*/
  $rfc_rc = install();
  if( $rfc_rc != SAPRFC_OK )
    exit;

  /*------------------------------------------------------------------*/
  /* Wait for Remote Function Call                                    */
  /*------------------------------------------------------------------*/
  do
  {
    $tbuf = "\n\nWait for next RFC call .....";
    TRFC_trace($tbuf);

    $rfc_rc = saprfc_trfc_dispatch($rfc_handle,$GLOBAL_FCE_LIST);

    $tbuf = sprintf("\n<==  RfcDispatch              rfc_rc = %d", $rfc_rc);
    TRFC_trace($tbuf);

  } while ($rfc_rc == SAPRFC_OK);

  if ($trace_fp != false)
    fclose($trace_fp);

  exit;
}

/*====================================================================*/
/*                                                                    */
/* Install the offered functions for RFC                              */
/*                                                                    */
/*====================================================================*/
function install()
{
  global $GLOBAL_FCE_LIST, $rfc_handle;
  
  $tbuf = "\n<==  RfcInstallTransactionControl";
  TRFC_trace($tbuf);
  
  saprfc_trfc_install ("TID_check", "TID_commit", "TID_rollback", "TID_confirm","USER_GLOBAL_SERVER");

  $tbuf = sprintf("\n<==  RfcInstallFunction '%s' rfc_rc = ", "STFC_WRITE_TO_TCPIC");
  TRFC_trace($tbuf);

  $def = array (
  			 array (
  				 "name"=>"RESTART_QNAME",
  				 "type"=>"IMPORT",
  				 "optional"=>"0",
  				 "def"=> array (
  					 array ("name"=>"","abap"=>"C","len"=>24,"dec"=>0)
  					)
  			),
  			 array (
  				 "name"=>"TCPICDAT",
  				 "type"=>"TABLE",
  				 "optional"=>"0",
  				 "def"=> array (
  					 array ("name"=>"LINE","abap"=>"C","len"=>72,"dec"=>0)
  					)
  			)
  		);
  $GLOBAL_FCE_LIST["STFC_WRITE_TO_TCPIC"] = saprfc_function_define($rfc_handle,"STFC_WRITE_TO_TCPIC",$def);

  return SAPRFC_OK;
}

/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION: %%USER_GLOBAL_SERVER                                 */
/*                                                                    */
/* Global server function for working with RfcGetName                 */
/*                                                                    */
/*====================================================================*/
function USER_GLOBAL_SERVER ($function_name)
{
  global $GLOBAL_FCE_LIST;

  return (isset($GLOBAL_FCE_LIST[$function_name]) ? $GLOBAL_FCE_LIST[$function_name] : 0);

}

/*====================================================================*/
/*                                                                    */
/* RFC-FUNCTION: STFC_WRITE_TO_TCPIC                                  */
/*                                                                    */
/* Received data from internal table will be write in a file          */
/*                                                                    */
/*====================================================================*/
function STFC_WRITE_TO_TCPIC ($fce)
{
  global $rfc_handle, $data_file;

  $tbuf = sprintf("\n\nStart Function %s", "STFC_WRITE_TO_TCPIC");
  TRFC_trace($tbuf);

  $tbuf = "\n<==  RfcGetAttributes         rc = ";
  TRFC_trace($tbuf);
  
  $rfc_attributes = saprfc_attributes ($rfc_handle);
  $tbuf = sprintf("%d", 0);
  TRFC_trace($tbuf);
  if ($rfc_attributes == false)
    rfc_error("RfcGetAttributes");

  $tbuf  = "\n\nAttributes of this RFC connection";
  $tbuf .= "\n---------------------------------";
  $tbuf .= sprintf ("\nDestination            :  %s", $rfc_attributes[dest]);
  $tbuf .= sprintf ("\nMy Host                :  %s", $rfc_attributes[own_host]);

  if ($rfc_attributes[rfc_role] == RFC_ROLE_CLIENT)
  {
    if ($rfc_attributes[partner_type] == RFC_SERVER_EXT)
      $tbuf .= sprintf("\nServer Program Name    :  %s", $rfc_attributes[partner_host]);
    elseif ($rfc_attributes[partner_type] == RFC_SERVER_EXT_REG)
      $tbuf .= sprintf("\nServer Program ID      :  %s", $rfc_attributes[partner_host]);
    else
      $tbuf .= sprintf("\nPartner Host           :  %s", $rfc_attributes[partner_host]);
  }
  else
    $tbuf.= sprintf("\nPartner Host           :  %s", $rfc_attributes[partner_host]);

  $tbuf.= sprintf("\nSystem No.             :  %s", $rfc_attributes[systnr]);
  $tbuf.= sprintf("\nSystem Name            :  %s", $rfc_attributes[sysid]);
  $tbuf.= sprintf("\nClient                 :  %s", $rfc_attributes[client]);
  $tbuf.= sprintf("\nUser                   :  %s", $rfc_attributes[user]);
  $tbuf.= sprintf("\nLanguage               :  %s", $rfc_attributes[language]);
  $tbuf.= sprintf("\nISO-Language           :  %s", $rfc_attributes[ISO_language]);

  if ($rfc_attributes[trace] == 'X')
    $tbuf .= sprintf("\nRFC Trace              :  ON");
  else
    $tbuf .= sprintf("\nRFC Trace              :  OFF");

  $tbuf .= sprintf("\nMy Codepage            :  %s", $rfc_attributes[own_codepage]);
  $tbuf .= sprintf("\nPartner Codepage       :  %s", $rfc_attributes[partner_codepage]);

  if ($rfc_attributes[rfc_role] == RFC_ROLE_CLIENT)
    $tbuf .= sprintf("\nRFC Role               :  External RFC Client");
  else if ($rfc_attributes[own_type] == RFC_SERVER_EXT)
    $tbuf .= sprintf("\nRFC Role               :  External RFC Server, started by SAP gateway");
  else
    $tbuf .= sprintf("\nRFC Role               :  External RFC Server, registered at SAP gateway");

  $tbuf .= sprintf("\nRFC Library Release    :  %s", $rfc_attributes[own_rel]);

  if ($rfc_attributes[partner_type] == RFC_SERVER_R3)
    $tbuf .= sprintf("\nRFC Partner            :  SAP R/3");
  elseif ($rfc_attributes[partner_type] == RFC_SERVER_R2)
    $tbuf .= sprintf("\nRFC Partner            :  SAP R/2");
  elseif ($rfc_attributes[rfc_role] == RFC_ROLE_CLIENT)
  {
    if ($rfc_attributes[partner_type] == RFC_SERVER_EXT)
      $tbuf .= sprintf("\nRFC Partner            :  External RFC Server, started by SAP gateway");
    else
      $tbuf .= sprintf("\nRFC Partner            :  External RFC Server, registered at SAP gateway");
  }
  else
    $tbuf .= sprintf("\nRFC Partner            :  External RFC Client");


  $tbuf .= sprintf("\nPartner System Release :  %s", $rfc_attributes[partner_rel]);
  $tbuf .= sprintf("\nR/3 Kernel Release     :  %s", $rfc_attributes[kernel_rel]);
  $tbuf .= sprintf("\nCPI-C Conversation ID  :  %s\n", $rfc_attributes[CPIC_convid]);
  TRFC_trace($tbuf);
  /* Put the name of data file in trace file */
  $tbuf = sprintf("\nDATA FILE:  '%s'", $data_file);
  TRFC_trace($tbuf);

  $rc = write_itab_to_file($fce, $data_file, "TCPICDAT");

  if ($rc)
  {
    return ("CANNOT_WRITE_DATA");
  }
  return true;

}


/*--------------------------------------------------------------------*/
/* TID_CHECK-Function for transactional RFC                           */
/*--------------------------------------------------------------------*/
function TID_check($tid)
{
  global $data_file, $TransID;

  $tbuf = sprintf("\n\nStart Function TID_CHECK      TID = %s", $tid);
  TRFC_trace($tbuf);
  $TransID = $tid;

  /* Check TID from TID Management */
  $tid_rc = check_TID($tid, $data_file);

  if ($tid_rc == TID_OK)
  {
    create_file_name($data_file);
    $data_file.=".dat";
    return (0);
  }

  if ($tid_rc == TID_FOUND)
    return (1);

  if ($tid_rc == TID_ERROR_LOCK)
    function_abort("Check_TID: Lock the TID-Management failed");
  else
    function_abort("Check_TID: Update the TID-Management failed");

  return (1);
}

/*--------------------------------------------------------------------*/
/* TID_COMMIT-Function for transactional RFC                          */
/*--------------------------------------------------------------------*/
function TID_commit($tid)
{
  global $data_file, $TransID;

  $tbuf = sprintf("\n\nStart Function TID_COMMIT     TID = %s", $tid);
  TRFC_trace($tbuf);
  $TransID = $tid;

  /* Commit TID from TID Management */
  $tid_rc = update_TID($tid, EXECUTED, $data_file);

  if ($tid_rc == TID_OK)
    return;

  if ($tid_rc == TID_ERROR_LOCK)
    function_abort ("Commit_TID: Lock the TID-Management failed");
  else
    function_abort("Commit_TID: Update the TID-Management failed");

  return;
}


/*--------------------------------------------------------------------*/
/* CONFIRM-Function for transactional RFC                             */
/*--------------------------------------------------------------------*/
function TID_confirm($tid)
{
  global $data_file, $TransID;

  sprintf(tbuf, "\n\nStart Function TID_CONFIRM    TID = %s", $tid);
  TRFC_trace($tbuf);
  $TransID = $tid;

  /* Confirm TID from TID Management */
  $tid_rc = update_TID($tid, CONFIRMED, $data_file);

  if ($tid_rc == TID_OK)
    return;

  if ($tid_rc == TID_ERROR_LOCK)
    function_abort("Confirm_TID: Lock the TID-Management failed");
  else
    function_abort("Confirm_TID: Update the TID-Management failed");

  return;
}


/*--------------------------------------------------------------------*/
/* TID_ROLLBACK-Function for transactional RFC                        */
/*--------------------------------------------------------------------*/
function TID_rollback($tid)
{
  global $data_file, $TransID;

  $tbuf = sprintf("\n\nStart Function TID_ROLLBACK   TID = %s", $tid);
  TRFC_trace($tbuf);
  $TransID = $tid;

  /* Rollback TID from TID Management */
  $tid_rc = update_TID($tid, ROLLBACK, $data_file);

  if ($tid_rc == TID_OK)
    return;

  if ($tid_rc == TID_ERROR_LOCK)
    function_abort("Rollback_TID: Lock the TID-Management failed");
  else
    function_abort("Rollback_TID: Update the TID-Management failed");

  return;
}


/*--------------------------------------------------------------------*/
/* Initialize the TID-Management                                      */
/*--------------------------------------------------------------------*/
function init_TID()
{
  global $tid_file, $lock_file, $TransID;
  
  /* Read TID-file for check and update */
  $fp = @fopen($tid_file, "rb+");
  if ($fp == false)
  {
    /* TID-Management not existent */
    $fp = fopen($tid_file, "ab");
    if ($fp == false)
    {
      /* ERROR: Create file for TID-Management failed */
      unlock_TID($lock_file);
      $tbuf = "Create_TID: Cannot create the TID-Management";
      TRFC_trace($tbuf);
      exit;
    }

    $tbuf = "***   TID-MANAGEMENT FOR TRANSACTIONAL RFC (Server Program)   ***\n";
    if (fputs($fp, $tbuf) == false)
    {
      /* ERROR: Write in TID-Management failed */
      unlock_TID($lock_file);
      fclose($fp);
      $tbuf = "Check_TID: Update the TID-Management failed";
      TRFC_trace($tbuf);
      exit;
    }
  }
  else
  {
    while (!feof($fp))
    {
        $tbuf = fgets ($fp,LINE_SIZE+1);
        if (substr ($tbuf,52,7) == "CREATED")
        {
           $TransID = substr ($tbuf,26,24);
           $tid_rc = update_TID($TransID, ROLLBACK, $data_file);
           if ($tid_rc == TID_OK)
               continue;
           if ($tid_rc == TID_ERROR_LOCK)
             $tbuf = "Check_TID: Lock the TID-Management failed";
           else
             $tbuf = "Check_TID: Update the TID-Management failed";
           TRFC_trace($tbuf);
	       exit;
         }
    }
  }
  return;
}



/*--------------------------------------------------------------------*/
/* Check and Update TID in the TID-Management                         */
/*--------------------------------------------------------------------*/
function check_TID($tid, $datafile)
{
  global $lock_file, $tid_file;
  
  /* Try to lock TID-Management */
  if (lock_TID($lock_file, $tid))
    return TID_ERROR_UPDATE;

  /* Read TID-file for check and update */
  $fp = fopen($tid_file, "rb+");
  if ($fp == false)
    $tidrc = TID_ERROR_UPDATE;

  while (!feof($fp))
  {
    $tbuf = fgets ($fp,LINE_SIZE+1);
    if (substr ($tbuf,26,24) == $tid && substr ($tbuf,52,8) != "ROLLBACK")
      break;
  }

  if (!feof($fp))
  {
    /* Previous TID found: Inform the RFC-Library about EXECUTED RFC-functions */
    unlock_TID($lock_file);
    fclose($fp);
    return TID_FOUND;
  }

  /* Write new TID at the end of the tid_file */
  $tid_state = CREATED;
  if (write_TID($fp, $tid, $tid_state, $datafile))
    $tidrc = TID_ERROR_UPDATE;
  else
    $tidrc = TID_OK;

  /* Unlock TID-Management */
  unlock_TID($lock_file);
  fclose($fp);
  return $tidrc;
}


/*--------------------------------------------------------------------*/
/* Update TID in the TID-Management                                   */
/*--------------------------------------------------------------------*/
function update_TID($tid,$tid_state,$datafile)
{
  global $lock_file, $tid_file;

  $offset = 0;
  /* Try to lock TID-Management */
  if (lock_TID($lock_file, $tid))
    return TID_ERROR_UPDATE;

  /* Open TID-file for update */
  $fp = fopen($tid_file, "rb+");
  if ($fp == false)
  {
    /* ERROR: Open TID-Management failed */
    unlock_TID($lock_file);
    return TID_ERROR_UPDATE;
  }

  while (!feof($fp))
  {
    $tbuf = fgets ($fp,LINE_SIZE+1);
    echo (substr ($tbuf,26,24)."\n");
    echo (substr ($tbuf,52,8)."\n");
    
    if (substr ($tbuf,26,24) == $tid && substr ($tbuf,52,8) != "ROLLBACK")
      break;
    $offset = $offset + strlen($tbuf);
  }

  if ((!feof ($fp)) &&
      (fseek($fp, $offset, SEEK_SET) == 0) &&
      (write_TID($fp, $tid, $tid_state, $datafile) == 0))
    $tidrc = TID_OK;
  else
    /* ERROR: Update TID-Management failed */
    $tidrc = TID_ERROR_UPDATE;

  /* Unlock TID-Management */
  unlock_TID($lock_file);
  fclose($fp);
  return $tidrc;
}


/*--------------------------------------------------------------------*/
/* Write TID in the TID-Management                                    */
/*--------------------------------------------------------------------*/
function write_TID($fp, $tid, $tid_state, $datafile)
{
  global $datetime;
  $datetime = date("D M j G:i:s Y");

  switch($tid_state)
  {
    case CREATED:
      $tbuf = sprintf("%s  %s  CREATED    %s\n", $datetime, $tid, $datafile);
      break;

    case EXECUTED:
      $tbuf = sprintf("%s  %s  EXECUTED   %s\n", $datetime, $tid, $datafile);
      break;

    case CONFIRMED:
      $tbuf = sprintf("%s  %s  CONFIRMED  %s\n", $datetime, $tid, $datafile);
      break;

    case ROLLBACK:
      $tbuf = sprintf("%s  %s  ROLLBACK   %s\n", $datetime, $tid, $datafile);
      break;
  }

  if (fputs($fp,$tbuf) == false)
    return 1;
  else
    return 0;
}


/*--------------------------------------------------------------------*/
/* Lock the TID-Management                                            */
/*--------------------------------------------------------------------*/
function lock_TID($lockfile, $tid)
{
  global $datetime, $lock_fd;
  
  $try_lock = 60;

  while ($try_lock)
  {
    $lock_fd = fopen ($lockfile,"w");
    if (($lock_fd = fopen ($lockfile,"w")) == false)
      return 1;

    if (flock ($lock_fd,LOCK_EX) == false )
    {
      fclose($lock_fd);
      sleep(1);
      $try_lock--;
    }
    else
    {
      $datetime = date("D M j G:i:s Y");
      $tbuf = sprintf("%s  %s  EXECUTING  \n", $datetime, $tid);
      fwrite($lock_fd, $tbuf, LINE_SIZE);
      return 0;
    }
  }
  return 1;

}


/*--------------------------------------------------------------------*/
/* Unlock the TID-Management                                          */
/*--------------------------------------------------------------------*/
function unlock_TID($lockfile)
{
  global $lock_fd;
  fclose ($lock_fd);
  unlink($lockfile);
  return 1;
}


/*--------------------------------------------------------------------*/
/* Move internal table to file                                        */
/*--------------------------------------------------------------------*/
function write_itab_to_file($fce, $data_file, $itab_name)
{
  /* Open data_file */
  $fp = fopen($data_file, "a");
  if ($fp == false)
    return 1;

  $it_fill = saprfc_table_rows ($fce,$itab_name);
  $tbuf = sprintf("Table Name=%s\nTable Line=%u\nTable Fill=%u\n",
	  $itab_name, 0, $it_fill);
   
  if ( fwrite($fp,$tbuf,strlen($tbuf)) < strlen($tbuf) )
    function_abort("Cannot write data in received file");

  for ($k = 1; $k<=$it_fill ; $k++)
  {
    $ptr = saprfc_table_read ($fce,$itab_name,$k);

    $tbuf = $ptr[LINE]."\n";
    fwrite($fp, $tbuf);
  }

  /* Close data_file */
  fclose($fp);

  return 0;
}


/*--------------------------------------------------------------------*/
/* Error Cleanup because of an RFC-Error                              */
/* Because of Windows DLL function must not be defined as static      */
/*--------------------------------------------------------------------*/
function rfc_error($operation)
{
  global $trace_fp, $rfc_handle;

  $tbuf = "\n<==  RfcLastErrorEx\n";

  $tbud .= sprintf("\nRFC Call/Exception: %s\n", $operation);
  TRFC_trace($tbuf);

  $tbuf = saprfc_error();
  TRFC_trace($tbuf);

  $tbuf = "\n<==  RfcClose\n";
  TRFC_trace($tbuf);

  saprfc_close ($rfc_handle);

  if ($trace_fp != false)
    fclose($trace_fp);

  exit;
}


/*--------------------------------------------------------------------*/
/* Issue RfcAbort with Abort Text because of an Application Error     */
/*--------------------------------------------------------------------*/
function function_abort($atext)
{
  global $trace_fp;
  
  $tbuf = sprintf ("\nRfcAbort   '%s'\n", $atext);
  TRFC_trace($tbuf);


  if ($trace_fp != NULL)
    fclose($trace_fp);

  exit;
}


/*--------------------------------------------------------------------*/
/* Write Trace Info into trace_file                                   */
/*--------------------------------------------------------------------*/
function TRFC_trace($text)
{
  global $trace_fp, $stderr;

  if ($trace_fp != false)
  {
    fputs($trace_fp, $text);
    fflush($trace_fp);
  }
  fputs ($stderr,sprintf("%s", $text));
  fflush($stderr);

  return;
}


/*--------------------------------------------------------------------*/
/* Create file name                                                   */
/*--------------------------------------------------------------------*/
function create_file_name(&$filename)
{
  global $file_nr, $working_dir;
  
  if ($file_nr == 0)
  {
    $file_nr = time();
    $znr = intval ($file_nr/2);
    srand($znr);
    $file_nr = getmypid() + rand();
  }
  else
    $file_nr++;

  $filename = $working_dir.sprintf ("tr%u", $file_nr);

  return;
}


/*--------------------------------------------------------------------*/
/* Output help for starting program                                   */
/*--------------------------------------------------------------------*/
function help()
{
  echo( "\n"                                                          );
  echo( "Syntax for start and run in register mode:                \n");
  echo( "                                                          \n");
  echo( "  trfcserv.php [options]                                  \n");
  echo( "                                                          \n");
  echo( "  with                                                    \n");
  echo( "  options = -a<program ID> e.g.  <own host name>.trfcserv \n");
  echo( "          = -g<SAP gateway host name>        e.g.  hs0311 \n");
  echo( "          = -x<SAP gateway service>          e.g. sapgw53 \n");
  echo( "          = -t             RFC-Trace on                   \n");
  echo( "                                                          \n");
  return;
}

//---trfcserv.h---------------------------------------------------------------


define ("RFC_ROLE_CLIENT",       'C');
define ("RFC_ROLE_SERVER",       'S');
define ("RFC_SERVER_R2",         '2');
define ("RFC_SERVER_R3",         '3');
define ("RFC_SERVER_EXT",        'E');
define ("RFC_SERVER_EXT_REG",    'R');



/*--------------------------------------------------------------------*/
/* Defines                                                            */
/*--------------------------------------------------------------------*/

define ("LINE_SIZE", 256);

define ("TID_OK",0);		                /* TID function OK       */
define ("TID_FOUND",1);      	            /* TID already exists    */
define ("TID_ERROR_LOCK",2); 	            /* TID lock failed       */
define ("TID_ERROR_UPDATE",3);              /* TID update failed     */

/* State of TID in the TID Management */
define ("CREATED",0);                       /* Transaction created   */
define ("EXECUTED",1);		                /* Transaction executed  */
define ("CONFIRMED",2);		                /* Transaction confirmed */
define ("ROLLBACK",3); 		                /* Transaction rollback  */

/*--------------------------------------------------------------------*/
/* Declarations                                                       */
/*--------------------------------------------------------------------*/

$file_nr = 0;

$tid_file="";
$data_file="";
$lock_file="";
$trace_file="";
$working_dir="";

$trace_fp = false;
$lock_fd = false;

$datetime = "";

$rfc_handle = false;
$TransID = "";


//-----------------------------------------------------------------------------

  main($argc,$argv);

//-----------------------------------------------------------------------------
?>

