<?
// SAPRFC_TEST.PHP - $Id: saprfc_test.php,v 1.7 2004/07/15 05:42:52 koucky Exp $
// Test script for SAPRFC extension module for PHP 4 and PHP 5
// 
// Features:
//      - similar function as "Single Test" in SE37 transaction (Function Builder)
//      - support login to application server or to logon group (using load balancing)
//      - login data are saved in cookie on client side
//      - select of function module from list of available RFC functions in target R/3 (* convention is supported)
//      - display R/3 system information (system id, version R/3, ....) after succesfull login
//      - possibility to set import parameters and fill internal tables
//      - generate fully functional PHP script calling of the function module
//      - call selected RFC function and show results


function print_header ($title)
{
  echo "
  <html>
  <head>
  <title>$title</title>
  <style type=\"text/css\">
     body {	background-color: #FFFFFF; font-family : Verdana, Geneva, Arial, Helvetica, sans-serif; font-size : 10pt; }
     td {font-family : Verdana, Geneva, Arial, Helvetica, sans-serif; font-size : 10pt; }
     .text1 { color: #0000A0; font-size : 14pt;}
     .text2 { color: Red; font-size : 9pt;}
     .text3 { font-size : 8pt;}
     .text4 { color: Red; }
     .td1 {font-size : 12pt; background-color : #0000a0; color: #ffffff;}
     .td2 {font-size: 10pt; background-color : #FFFFFF;}
     .td3 {font-size : 9pt; background-color : #9FC1EA; color:#0000a0;}
     .td4 {font-size : 12pt; background-color : red; color:#ffffff;}
     .td5 {font-size: 10pt; background-color : #ffffff;}
     .td6 {font-size : 8pt; background-color : #ffa0a0; color:#000000;}
     .table1 {background-color : #000000; }
     .table2 {background-color : Red; }
  </style>
  </head>
  <body>
  <span class=\"text1\">
  <b><a  href=\"http://saprfc.sourceforge.net\">SAPRFC</a> test script</b><br>
  </span>
  <span class=\"text2\">
  <b>extension module for PHP 4 and PHP 5</b><br><br><br>
  </span>
  ";
}


function print_footer ()
{
  global $RFC_SYSTEM_INFO;

  if ($RFC_SYSTEM_INFO != "")
     echo ("<p align='center' class=\"text3\">$RFC_SYSTEM_INFO\n");
  echo "</body></html>";
}

function print_window ($title,$content,$bottom)
{
  echo ("<table class='table1' align='center' border=0 cellpadding=0 cellspacing=0 width=60%>\n");
  echo ("<tr><td><table border=0 cellpadding=3 cellspacing=1 width=100%>\n");
  echo ("<tr><td class=\"td1\">\n");
  echo ("<span align=center><b>$title</b></td></tr>\n");
  echo ("<tr><td class=\"td2\">\n");
  echo ("$content</td></tr>\n");
  if ($bottom)
  {
     echo ("<tr><td class=\"td3\">\n");
     echo ("$bottom</td></tr>\n");
  }
  echo ("</table></td></tr></table>\n");
}

function print_login_form ($ASHOST, $SYSNR, $CLIENT, $USER, $PASSWD, $GWHOST,
                           $GWSERV,  $MSHOST, $R3NAME, $GROUP,
                           $LANG, $TRACE, $LCHECK, $CODEPAGE)
{
    $form  = "<form name=login method=post action=saprfc_test.php>\n";
    $form .= "<input type=hidden name=p value=login>\n";
    $form .= "<table>\n";
    $form .= "<tr><td><b>Application server:</b></td><td><input type=text size=30 name=ASHOST value=\"$ASHOST\"></td></tr>\n";
    $form .= "<tr><td><b>System number:</b></td><td><input type=text size=2 name=SYSNR value=\"$SYSNR\"></td></tr>\n";
    $form .= "<tr><td><b>Client:</b></td><td><input type=text size=3 name=CLIENT value=\"$CLIENT\"></td></tr>\n";
    $form .= "<tr><td><b>User:</b></td><td><input type=text size=12 name=USER value=\"$USER\"></td></tr>\n";
    $form .= "<tr><td><b>Password:</b></td><td><input type=password size=12 name=PASSWD value=\"$PASSWD\"></td></tr>\n";
    $form .= "<tr><td><b>Message server:</b></td><td><input type=text size=30 name=MSHOST value=\"$MSHOST\"></td></tr>\n";
    $form .= "<tr><td><b>R/3 system name:</b></td><td><input type=text size=3 name=R3NAME value=\"$R3NAME\"></td></tr>\n";
    $form .= "<tr><td><b>Logon group:</b></td><td><input type=text size=10 name=GROUP value=\"$GROUP\"></td></tr>\n";
    $form .= "<tr><td><b>Language:</b></td><td><input type=text size=2 name=LANG value=\"$LANG\"></td></tr>\n";
    $checked = ($TRACE) ? "checked" : "";
    $form .= "<tr><td colspan=2><input type=checkbox name=TRACE value=1 $checked> trace (RFC debug info to syslog)</td></tr>\n";
    $checked = ($LCHECK) ? "checked" : "";
    $form .= "<tr><td colspan=2><input type=checkbox name=LCHECK value=1 $checked> authenticate user login data (if not set, the RFC connection is opened only)</td></tr>\n";
    $form .= "<tr><td><b>SAP Codepage:</b></td><td><input type=text size=4 name=CODEPAGE value=\"$CODEPAGE\"></td></tr>\n";
    $form .= "<tr><td colspan=2 align=center><input type=submit value=\"Login\">&nbsp;<input type=reset value=\"Reset\"></td></tr>\n";
    $form .= "</table></form>\n";
    $form .= "<p align=center><span class=\"text4\"><b>Warning: The password is send as the clear text !!! <br>Don't use this script in the Internet environment. <br>Don't login to the productive system. </b></span>\n";
    print_header ("Login to SAP R/3");
    print_window ("Login to SAP R/3",$form,"");
    print_footer ();
}

function print_select_form ($function, $last_entries, $functions_list)
{

    $form  = "<form name=select method=post action=saprfc_test.php>\n";
    $form .= "<input type=hidden name=p value=select_do>\n";
    $form .= "<table>\n";
    $form .= "<tr><td><b>Function module:</b></td><td><input type=text size=30 name=function value=\"$function\"></td></tr>\n";
    if (is_array ($last_entries))
    {
       $form .= "<tr><td colspan=2>or\n";
       $form .= "<tr><td><b>Last entries:</b></td><td><select size=1 name=entry>\n";
       $form .= "<option value = \"\"></option>\n";
       for ($i=0;$i<count($last_entries);$i++)
          $form .= "<option value = \"".$last_entries[$i]."\">".$last_entries[$i]."</option>\n";
       $form .= "</select></td></tr>\n";
    }
    if (is_array ($functions_list))
    {
       $form .= "<tr><td colspan=2>or\n";
       $form .= "<tr><td><b>List of functions:</b></td><td><select size=1 name=list>\n";
       $form .= "<option value = \"\"></option>\n";
       for ($i=0;$i<count($functions_list);$i++)
          $form .= "<option value = \"".$functions_list[$i]."\">".$functions_list[$i]."</option>\n";
       $form .= "</select></td></tr>\n";
    }
    $form .= "<tr><td colspan=2 align=center><input type=submit value=\"Select function module\">&nbsp;<input type=reset value=\"Reset\"></td></tr>\n";
    $form .= "</table></form>\n";
    print_header ("Selection of function module");
    print_window ("Selection of function module",$form,"<a href=\"saprfc_test.php?p=newlogin\">New login</a>");
    print_footer ();
}


function print_error_message ($title, $message, $debug)
{
  print_header ($title);

  echo ("<table align='center' class='table2' border=0 cellpadding=0 cellspacing=0 width=60%>\n");
  echo ("<tr><td><table border=0 cellpadding=3 cellspacing=1 width=100%>\n");
  echo ("<tr><td class=\"td4\">\n");
  echo ("<span align='center'><b>$title</b></td></tr>\n");
  echo ("<tr><td class=\"td5\">\n");
  echo ("<b>Message: </b>$message</td></tr>\n");
  echo ("<tr><td class=\"td6\">\n");
  echo ("$debug</td></tr>");
  echo ("</table></td></tr></table>");
  print_footer ();
}

function show_parameter ($name,$optional, $def, $value)
{
   if ( $def[0]["name"] == "" )
   {
       $form  = sprintf ("<tr><td colspan=2><b>%s</b> &nbsp; &nbsp; <input name=\"RFC_%s\" type=text size=%d value=\"%s\">",$name,$name,$def[0]["len"],$value);
       $form .= sprintf (" %s(%d)</td></tr>\n",$def[0]["abap"],$def[0]["len"]);
   }
   else
   {
       $form = "<tr><td valign=top><b>$name</b></td><td>\n<table colspacing=0 colpadding=0>\n<tr>";
       for ($i=0;$i<count($def);$i++)
          $form .= sprintf ("<td align=center>%s<br>%s(%d)</td>\n",$def[$i]["name"],$def[$i]["abap"],$def[$i]["len"]);
       $form .= "</tr>\n<tr>\n";
       for ($i=0;$i<count($def);$i++)
       {
          $size = $def[$i]["len"] < 80 ? $def[$i]["len"] : 80;
          $form  .= sprintf ("<td align=center><input name=\"RFC_%s[%s]\" type=text size=%d value=\"%s\"></td>\n",$name,$def[$i]["name"],$size,$value[$i]);
       }
       $form .= "</tr>\n</table>\n</td></tr>\n";
   }
   return ($form);
}

function show_parameter_out ($name, $def, $value)
{
   if ( $def[0]["name"] == "" )
   {
       $form  = sprintf ("<tr><td><b>%s</b></td><td>%s</td></tr>\n",$name,$value);
   }
   else
   {
       $form = "<tr><td valign=top><b>$name</b></td>\n<td>\n<table colspacing=0 colpadding=0>\n<tr>\n";
       for ($i=0;$i<count($def);$i++)
          $form .= sprintf ("<td align=center>%s</td>\n",$def[$i]["name"]);
       $form .= "</tr>\n<tr>\n";
       for ($i=0;$i<count($def);$i++)
          $form  .= sprintf ("<td>%s</td>\n",$value[$def[$i]["name"]]);
       $form .= "</tr>\n</table>\n</td></tr>\n";
   }
   return ($form);
}

function show_table ($name, $def, $value)
{
   $form = "<tr><td colspan=2>\n<table colspacing=0 colpadding=0>\n<tr>\n";
   for ($i=0;$i<count($def);$i++)
       $form .= sprintf ("<td align=center>%s<br>%s(%d)</td>\n",$def[$i]["name"],$def[$i]["abap"],$def[$i]["len"]);
   $form .= "</tr>\n";
   $form .= sprintf ("<input type=hidden name=%s value=\"%s\">\n","RFC_TABLE_".$name,urlencode(serialize($value)));
   for ($j=0;$j<count($value);$j++)
   {
     $form .= "<tr>\n";
     for ($i=0;$i<count($def);$i++)
       $form .= sprintf ("<td>%s</td>\n",$value[$j][$def[$i]["name"]]);
     $form .= "</tr>\n";
   }
   for ($i=0;$i<count($def);$i++)
   {
     $size = $def[$i]["len"] < 80 ? $def[$i]["len"] : 80;
     $form  .= sprintf ("<td align=center><input name=\"RFC_%s[%s]\" type=text size=%d></td>\n",$name,$def[$i]["name"],$size);
   }
   $form .= "</tr></table></td></tr>\n";
   $form .= "<tr><td><input type=submit name=action value=\"Append $name\"></td></tr>\n";
   return ($form);
}

function show_table_out ($name, $def, $value)
{
   $form = "<tr><td colspan=2>\n<table colspacing=0 colpadding=0>\n<tr>";
   for ($i=0;$i<count($def);$i++)
       $form .= sprintf ("<td colspan=2>%s</td>\n",$def[$i]["name"]);
   $form .= "</tr>\n";
   for ($j=0;$j<count($value);$j++)
   {
     $form .= "<tr>\n";
     for ($i=0;$i<count($def);$i++)
       $form .= sprintf ("<td colspan=2>%s</td>\n",$value[$j][$def[$i]["name"]]);
     $form .= "</tr>\n";
   }
   $form .= "</table>\n</td></tr>\n";
   return ($form);
}

//-----------------------------------------------------------------------------
// MAIN
//-----------------------------------------------------------------------------

if ( ini_get("register_globals") == false )
{
   if ( isset ($_COOKIE) ) {
       foreach ($_COOKIE as $var=>$val) {
           $$var = $val;
       }
   } elseif ( isset ($HTTP_COOKIE_VARS) ) {
       foreach ($HTTP_COOKIE_VARS as $var=>$val) {
           $$var = $val;
       }
   }
   
   if ( isset ($_POST) ) {
       foreach ($_POST as $var=>$val) {
           $$var = $val;
       }
   } elseif ( isset ($HTTP_POST_VARS) ) {
       foreach ($HTTP_POST_VARS as $var=>$val) {
           $$var = $val;
       }
   }

   if ( isset ($_GET) ) {
       foreach ($_GET as $var=>$val) {
           $$var = $val;
       }
   } elseif ( isset ($HTTP_GET_VARS) ) {
       foreach ($HTTP_GET_VARS as $var=>$val) {
           $$var = $val;
       }
   }
}

if (! extension_loaded ("saprfc"))
{
   print_error_message ( "SAPRFC extension not loaded",
                         "This script use SAPRFC extension module for PHP and the extension isn't loaded. You can download ".
                         "it with installation instructions from <a href=\"saprfc.sourceforge.net\">".
                         "http://saprfc.sourceforge.net</a>. If you have already the extension compiled and installed, check your ".
                         "php.ini configuration ",
                         ""
                          );
   exit;

}


if (isset($SAPRFC_LOGON))                           // if cookie with login data, decode it
{
    $SAPRFC_LOGON = urldecode($SAPRFC_LOGON);
    $l = unserialize ($SAPRFC_LOGON);                   // l =  array with login data (user,client, host....)
}
 
if (isset($SAPRFC_ENTRIES)) {
    $SAPRFC_ENTRIES = urldecode($SAPRFC_ENTRIES);       // cookie with last entries
    $e = unserialize ($SAPRFC_ENTRIES);                 // e =  array with last entries (selected funcions)
}

if (!isset ($p) ) $p="";

// if no cookie with login data exist or if newlogin request, show login page
if ( ! isset($SAPRFC_LOGON) && $p != "login" || $p == "newlogin" )
{
    if (! isset($l["ASHOST"]))             $l["ASHOST"] = "";
    if (! isset($l["SYSNR"]))              $l["SYSNR"] =  "";
    if (! isset($l["CLIENT"]))             $l["CLIENT"] = "";
    if (! isset($l["USER"]))               $l["USER"] =  "";
    if (! isset($l["PASSWD"]))             $l["PASSWD"] = "";
    if (! isset($l["GWHOST"]))             $l["GWHOST"] =  "";
    if (! isset($l["GWSERV"]))             $l["GWSERV"] =  "";
    if (! isset($l["MSHOST"]))             $l["MSHOST"] =  "";
    if (! isset($l["R3NAME"]))             $l["R3NAME"] =  "";
    if (! isset($l["GROUP"]))              $l["GROUP"] =  "";
    if (! isset($l["LANG"]))               $l["LANG"] =  "";
    if (! isset($l["TRACE"]))              $l["TRACE"] =  "";
    if (! isset($l["LCHECK"]))             $l["LCHECK"] =  "";
    if (! isset($l["CODEPAGE"]))           $l["CODEPAGE"] =  "";
    
    print_login_form ($l["ASHOST"], $l["SYSNR"], $l["CLIENT"], $l["USER"], $l["PASSWD"], $l["GWHOST"],
                      $l["GWSERV"],  $l["MSHOST"], $l["R3NAME"], $l["GROUP"],
                      $l["LANG"], $l["TRACE"], $l["LCHECK"], $l["CODEPAGE"]);
    exit;
}
else
{
    if ( $p == "login" )  // process data from login page
    {
        if (isset($ASHOST))             $l["ASHOST"] = $ASHOST;
        if (isset($SYSNR))              $l["SYSNR"] = $SYSNR;
        if (isset($CLIENT))             $l["CLIENT"] = $CLIENT;
        if (isset($USER))               $l["USER"] = $USER;
        if (isset($PASSWD))             $l["PASSWD"] = $PASSWD;
        if (isset($GWHOST))             $l["GWHOST"] = $GWHOST;
        if (isset($GWSERV))             $l["GWSERV"] = $GWSERV;
        if (isset($MSHOST))             $l["MSHOST"] = $MSHOST;
        if (isset($R3NAME))             $l["R3NAME"] = $R3NAME;
        if (isset($GROUP))              $l["GROUP"] = $GROUP;
        if (isset($LANG))               $l["LANG"] = $LANG;
        if (isset($TRACE))              $l["TRACE"] = $TRACE;
        if (isset($LCHECK))             $l["LCHECK"] = $LCHECK;
        if (isset($CODEPAGE))           $l["CODEPAGE"] = $CODEPAGE;
        $SAPRFC_LOGON = serialize ($l);       // set cookie variable
        $p = "select";                        // switch to selection of function module
    }
}

// login data avaible, can open connection to SAP R/3
$rfc = @saprfc_open ($l);

if (! $rfc )                    // if login failed, show error message
{
   if (isset ($l["use_load_balancing"]))
      print_error_message ( "Login error",
                            "Can't login to client ".$l["CLIENT"]." of the system ".$l["R3NAME"]." (Host: ".$l["MSHOST"].", Group: ".$l["GROUP"].") as user ".$l["USER"].". <a href=\"saprfc_test.php?p=newlogin\">New login</a>",
                            saprfc_error()
                          );
   else
      print_error_message ( "Login error",
                            "Can't login to client ".$l["CLIENT"]." and host ".$l["ASHOST"]." (System number: ".$l["SYSNR"].") as user ".$l["USER"].". <a href=\"saprfc_test.php?p=newlogin\">New login</a>",
                            saprfc_error()
                          );
   exit;
}

// if SAP codepage set, set codepage for opened rfc connection
//if ($l[codepage]!="")
//     saprfc_set_code_page ($rfc,$l[codepage]);


// save login data as cookie at the client side
setcookie ("SAPRFC_LOGON", urlencode($SAPRFC_LOGON),time()+7200);

// retrieve a system information from connected R/3
// use function module RFC_SYSTEM_INFO

// discover interface of function module and set resource $sysinfo_fce
$sysinfo_fce = @saprfc_function_discover ($rfc,"RFC_SYSTEM_INFO"); 
if ($sysinfo_fce) 
{
     // do RFC call to connected R/3 system
     $retval = @saprfc_call_and_receive ($sysinfo_fce); 
     if ($retval == SAPRFC_OK) 
     {
          // retrieve export (output) parametr RFCSI_EXPORT
          $sysinfo = saprfc_export ($sysinfo_fce,"RFCSI_EXPORT");
          $RFC_SYSTEM_INFO = sprintf ("system id: %s (%s), client=%03d, user=%s, application server=%s (%s,%s,%s), database=%s (%s)",
                                       $sysinfo["RFCSYSID"],$sysinfo["RFCSAPRL"],$l["CLIENT"],$l["USER"],$sysinfo["RFCHOST"], $sysinfo["RFCOPSYS"],
                                       $sysinfo["RFCIPADDR"],$sysinfo["RFCKERNRL"], $sysinfo["RFCDBHOST"], $sysinfo["RFCDBSYS"] );
     }
     // free allocated resources
     @saprfc_function_free ($sysinfo_fce);
}


if ( $p == "select_do" )            // action for selection of function module page
{
    if (isset ($list) && $list != "")                // if function from function list selected, get it
    {
        $function = $list;
        $p = "input";
    }
    elseif (isset ($entry) && $entry != "")           // if function from last entries selected, get it
    {
        $function = $entry;
        $p = "input";
    }
    else
    {                               // other, use function module RFC_FUNCTION_SEARCH to
                                    // get a list of RFC functions of target R/3
      $search_fce = saprfc_function_discover ($rfc,"RFC_FUNCTION_SEARCH");
      if ($search_fce)
      {
        saprfc_import ($search_fce,"FUNCNAME",$function);   // you can use asterix *, e.g. RFC_*
        saprfc_optional ($search_fce,"GROUPNAME",1);        // set optional parameters, the default values will be used
        saprfc_optional ($search_fce,"LANGUAGE",1);
        saprfc_table_init ($search_fce,"FUNCTIONS");        // init internal table named FUNCTIONS, equivalent of abap
                                                         // commands: refresh functions; clear functions;
        $retval = @saprfc_call_and_receive ($search_fce);   // rfc call
        $numrows = saprfc_table_rows ($search_fce,"FUNCTIONS");    // get number of rows internal table
        if ($numrows == 0)                               // no function found, show selection page again
            $p = "select";
        elseif ($numrows == 1)
        {                                                // exactly one function found, show input page
            $row = saprfc_table_read ($search_fce,"FUNCTIONS",1);
            $function = $row["FUNCNAME"];
            $p = "input";
        }
        else
        {                                                // retrive list of founded function to array
            for ($i=1;$i<=$numrows;$i++)                 // note: lines of internal tables are indexed from 1, (e.g 1,2,3....)
            {
                $row = saprfc_table_read ($search_fce,"FUNCTIONS",$i);
                $functions_list[$i-1] = $row["FUNCNAME"];
            }
            $p = "select";                               // show selection page again
        }
        saprfc_function_free ($search_fce);
      }
      else                                               // error, can't discover interface of RFC_FUNCTION_SEARCH
        $p = "select";
    }
}

if ( $p == "select" || $p == "")                         //  selection of the function module page
{                                                        //  show also if have login data from cookie (p=="")
   if (! isset ($function) ) $function="";
   if (! isset ($e) ) $e="";
   if (! isset ($functions_list) ) $functions_list="";

   print_select_form ($function, $e, $functions_list);
}

if ( $p == "input" )                                     // input values for import parameters or internal tables
{                                                        // of function module $function
   if (!isset ($action) ) $action = "";
   if ($action == "")                                    // if enter from selection page, set last entries cookie
   {
     if (! isset ($e) || ! is_array ($e)) $e = array();
     $t_e = $e;
     unset ($e);
     $e[0] = strtoupper($function);
     foreach ($t_e as $t_func) {
          if (strtoupper($function) != $t_func) $e[] = $t_func;
     } 
     unset ($t_e);
     $SAPRFC_ENTRIES=serialize($e);
     setcookie ("SAPRFC_ENTRIES", urlencode($SAPRFC_ENTRIES),time()+3600);
   }
   
   $fce = @saprfc_function_discover ($rfc,$function);      // discover interface of function module $function
   if (! $fce )
   {
      print_error_message ( "Discovering function module error",
                            "Can't discover interface of function module <b>$function</b>. Please check if the module is Remote-enable (transaction SE37, Atrributes).<a href=\"saprfc_test.php?p=select\">Other function module</a>",
                            saprfc_error()
                          );
      exit;
   }
   $def = @saprfc_function_interface($fce);               // retrieve definition of interface in array $def
                                                       // $def has following format:
   /* $def = array (
                      array ("name"=>"PARAMETER",      // name of import, export parameter or internal table
                             "type"=>"IMPORT",         // type: IMPORT, EXPORT, TABLE
                             "optional"=>0             // 1 - optional parameter
                             "def"=> typedef           // type definition (elementary SAP data type or structure)
                            ),
                      ............
                   )
             typedef = array (
                                 array ("name"=>"item1" "abap"=>"C", "len"=>30, "dec"=>0),
                                 array ("name"=>"item2" "abap"=>"I", "len"=>4, "dec"=>0),
                                 .....
                              )
   */
   
   if ($action == "Call $function")                     // switch to call/result page if Call button pressed
      $p = "call";
   elseif ($action == "Generate PHP")
   {
       Header ("Content-type: application/x-httpd-php-source\n");
       Header ("Content-Disposition: inline; filename=${function}.php\n");

       echo ("<?\n//RFC Call for $function\n");
       echo ("//Generated by saprfc_test.php (http://saprfc.sourceforge.net)\n\n");
       echo ("//Login to SAP R/3\n");
       echo ("\$login = array (");
       $first = true;
       foreach ($l as $key => $value)
       {
          if ($value == "") continue;
          if ($first)
             {$first = false; echo ("\n");}
          else
             echo (",\n");
          echo ("\t\t\t\"$key\"=>\"$value\"");
       }
       echo (");\n");
       echo ("\$rfc = saprfc_open (\$login );\n");
       echo ("if (! \$rfc ) { echo \"RFC connection failed\"; exit; }\n");
       echo ("//Discover interface for function module $function\n");
       echo ("\$fce = saprfc_function_discover(\$rfc,\"$function\");\n");
       echo ("if (! \$fce ) { echo \"Discovering interface of function module failed\"; exit; }\n");
       echo ("//It's possible to define interface manually. If you would like do it, uncomment following lines:\n");
       echo ("/*\$def = array (\n");
       for ($i=0;$i<count($def);$i++)
       {
         $interface = $def[$i];
         echo ("  \t\t\t array (\n");
         echo ("  \t\t\t\t \"name\"=>\"".$interface["name"]."\",\n");
         echo ("  \t\t\t\t \"type\"=>\"".$interface["type"]."\",\n");
         echo ("  \t\t\t\t \"optional\"=>\"".$interface["optional"]."\",\n");
         echo ("  \t\t\t\t \"def\"=> array (\n");
         $typedef = $interface["def"];
         for ($j=0; $j<count($typedef); $j++)
         {
             echo ("  \t\t\t\t\t array (\"name\"=>\"".$typedef[$j]["name"]."\",\"abap\"=>\"".$typedef[$j]["abap"]."\",\"len\"=>".$typedef[$j]["len"].",\"dec\"=>".$typedef[$j]["dec"].",\"offset\"=>".$typedef[$j]["offset"].")");
             if ($j < (count($typedef)-1) )
                 echo (",\n");
              else
                 echo ("\n");
         }
         echo ("  \t\t\t\t\t)\n");
         echo ("  \t\t\t)");
         if ($i < (count($def)-1) )
              echo (",\n");
         else
              echo ("\n");
       }
       echo ("  \t\t); \n");
       echo ("  \$fce = saprfc_function_define(\$rfc,\"$function\",\$def);\n");
       echo ("*/\n");
       echo ("//Set import parameters. You can use function saprfc_optional() to mark parameter as optional.\n");
       for ($i=0;$i<count($def);$i++)
       {
         $interface = $def[$i];
         if ($interface["type"] == "IMPORT")
         {
            $var="RFC_".$interface["name"];
            if (!isset ($$var) ) $$var="";
            if (! is_array ($$var))
               echo ("saprfc_import (\$fce,\"".$interface["name"]."\",\"".$$var."\");\n");
            else
            {
               echo ("saprfc_import (\$fce,\"".$interface["name"]."\", array (");
               $first = true;
               foreach ($$var as $key => $value)
               {
                      if ($first)
                        {$first = false;}
                      else
                         echo (",");
                      echo ("\"$key\"=>\"$value\"");
               }
               echo ("));\n");
            }
         }
       }
       echo ("//Fill internal tables\n");
       for ($i=0;$i<count($def);$i++)
       {
         $interface = $def[$i];
         if ($interface["type"] == "TABLE")
         {
             echo ("saprfc_table_init (\$fce,\"".$interface["name"]."\");\n");
             $var="RFC_TABLE_".$interface["name"];
             if (!isset ($$var) ) $$var="";
             $vararray = unserialize (urldecode($$var));
             if (is_array($vararray))
             {
               for ($j=0;$j<count($vararray);$j++)
               {
                  echo ("saprfc_table_append (\$fce,\"".$interface["name"]."\", array (");
                  $first = true;
                  foreach ($vararray[$j] as $key => $value)
                  {
                      if ($first)
                        {$first = false;}
                      else
                         echo (",");
                      echo ("\"$key\"=>\"$value\"");
                  }
                  echo ("));\n");
               }
            }
         }
       }
       echo ("//Do RFC call of function $function, for handling exceptions use saprfc_exception()\n");
       echo ("\$rfc_rc = saprfc_call_and_receive (\$fce);\n");
       echo ("if (\$rfc_rc != SAPRFC_OK) { if (\$rfc == SAPRFC_EXCEPTION ) echo (\"Exception raised: \".saprfc_exception(\$fce)); else echo (saprfc_error(\$fce)); exit; }\n");
       echo ("//Retrieve export parameters\n");
       for ($i=0;$i<count($def);$i++)
       {
         $interface = $def[$i];
         if ($interface["type"] == "EXPORT")
         {
            $var="RFC_".$interface["name"];
            if (!isset ($$var)) $$var = "";
            echo ("\$".$interface["name"]." = saprfc_export (\$fce,\"".$interface["name"]."\");\n");
         }
       }
       for ($i=0;$i<count($def);$i++)
       {
         $interface = $def[$i];
         if ($interface["type"] == "TABLE")
         {
            echo ("\$rows = saprfc_table_rows (\$fce,\"".$interface[name]."\");\n");
            echo ("for (\$i=1;\$i<=\$rows;\$i++)\n");
                 echo ("\t\$".$interface[name]."[] = saprfc_table_read (\$fce,\"".$interface[name]."\",\$i);\n");
         }
       }
       echo ("//Debug info\n"); 
       echo ("saprfc_function_debug_info(\$fce);\n"); 
       echo ("saprfc_function_free(\$fce);\n"); 
       echo ("saprfc_close(\$rfc);\n"); 

   }
   else
   {
      print_header ("Test of function module $function");

      $form = "<form name=test method=post action=\"saprfc_test.php\">\n";
      $form .= "<input name=p type=hidden value=input>\n";
      $form .= "<input name=function type=hidden value=\"$function\">\n";
      $form .= "<table colspacing=0 colpadding=0>\n";
      $form .= "<tr bgcolor=#D0D0D0><td colspan=2><b>IMPORT PARAMETERS</b></td></tr>\n";
      for ($i=0;$i<count($def);$i++)
      {
         $interface = $def[$i];
         if ($interface["type"] == "IMPORT")              // if import parameter
         {
            $var="RFC_".$interface["name"];               // prefix RFC_
            if (!isset ($$var) ) $$var="";
            $form .= show_parameter ($interface["name"],$interface["optional"],$interface["def"],$$var);
         }
      }
      for ($i=0;$i<count($def);$i++)
      {
         $interface = $def[$i];
         if ($interface["type"] == "TABLE")               // if internal table
         {
            $form .= "<tr bgcolor=#D0D0D0><td colspan=2><b>TABLE ".$interface["name"]."</b></td></tr>\n";
            $var="RFC_TABLE_".$interface["name"];
            if ( !isset ($$var) ) $$var="";
            $vararray = unserialize (urldecode($$var));    // get saved array of values from RFC_TABLE_*
            $var="RFC_".$interface["name"];
            if ($action=="Append ".$interface["name"])       // if Append button pressed, add new line to internal table (array)
                $vararray[]=$$var;
            $form .= show_table ($interface["name"],$interface["def"],$vararray);
         }
      }
      $form .= "<tr><td colspan=2><input type=submit name=action value=\"Call $function\">\n";
      $form .="<input type=submit name=action value=\"Generate PHP\"></td></tr>\n";
      $form .= "</table></form>\n";
      print_window ("Function module $function",$form,"<a href=\"javascript:history.back()\">Back</a>&nbsp;<a href=\"saprfc_test.php?p=newlogin\">New login</a>&nbsp;<a href=\"saprfc_test.php?p=select\">Select other function</a>");
      print_footer();
   }
}

if ( $p == "call" )              // set input parameters and tables, call function module, show results
{
      for ($i=0;$i<count($def);$i++)
      {
         $interface = $def[$i];
         if ($interface["type"] == "IMPORT")
         {
            $var="RFC_".$interface["name"];
            saprfc_import ($fce,$interface["name"],$$var);  // set import parameters
         }
         if ($interface["type"] == "TABLE")
         {
            saprfc_table_init ($fce,$interface["name"]);
            $var="RFC_TABLE_".$interface["name"];
            $vararray = unserialize (urldecode($$var));
            if (is_array($vararray))
            {
               for ($j=0;$j<count($vararray);$j++)
                    saprfc_table_append ($fce,$interface["name"],$vararray[$j]);
            }
         }
      }
      // rfc call function in connected R/3
      $retval = @saprfc_call_and_receive ($fce);
      if ( $retval != SAPRFC_OK  )
      {
        print_error_message ( "Call error",
                              "Calling of function module $function failed. See the error message below.<a href=\"saprfc_test.php?p=select\">Other function module</a>",
                              saprfc_error()
                            );
        exit;
      }

      print_header ("Results of function module $function");

      $form = "<table colspacing=0 colpadding=0>\n";
      $form .= "<tr bgcolor=#D0D0D0><td colspan=2><b>EXPORT PARAMETERS</b></td></tr>\n";
      for ($i=0;$i<count($def);$i++)
      {
         $interface = $def[$i];
         if ($interface["type"] == "EXPORT")  // show export parameters
         {
            $var = saprfc_export ($fce,$interface["name"]);
            $form .= show_parameter_out ($interface["name"],$interface["def"],$var);
         }
      }
      for ($i=0;$i<count($def);$i++)
      {
         $interface = $def[$i];
         if ($interface["type"] == "TABLE")  // show content of internal tables
         {
            $form .= "<tr bgcolor=#D0D0D0><td colspan=2><b>TABLE ".$interface["name"]."</b></td></tr>\n";
            unset ($vararray);
            $rows = saprfc_table_rows ($fce,$interface["name"]);
            for ($j=1;$j<=$rows;$j++)
                $vararray[] = saprfc_table_read($fce,$interface["name"],$j);
            $form .= show_table_out ($interface["name"],$interface["def"],$vararray);
         }
      }
      $form  .= "</table>";
      print_window ("Results of function module $function",$form,"<a href=\"javascript:history.back()\">Back</a>&nbsp;<a href=\"saprfc_test.php?p=newlogin\">New login</a>&nbsp;<a href=\"saprfc_test.php?p=select\">Select other function</a>");
      print_footer();
}

// free resources and close rfc connection
@saprfc_function_free($fce);
@saprfc_close($rfc);

?>
